#!/usr/bin/env python3
import subprocess
import sys
import os
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from pygdbmi.gdbcontroller import GdbController

TEST_PATH = "./build/tests/028-libuv-io-runner.t"
TIMEOUT = 30  # Increased timeout for busy machine
MAX_RUNS = 2000
PARALLEL_THREADS = 32 # Number of parallel test executions
OUTPUT_DIR = "/tmp/race_test_outputs"

os.chdir("/home/ruoso/devel/Building-Programming-Language-Interpreters")

# Create output directory
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Thread-safe flag to signal when a failure is found
failure_found = threading.Event()
failure_lock = threading.Lock()


def identify_thread_type(frames):
    """Identify what type of thread this is based on stack frames."""
    frame_funcs = [f.get('func', '') for f in frames]
    stack_text = ' '.join(frame_funcs)
    
    if 'interpreter_loop' in stack_text:
        if 'handle_read' in stack_text:
            return 'interpreter_loop (in handle_read)'
        elif 'handle_write' in stack_text:
            return 'interpreter_loop (in handle_write)'
        elif 'handle_start_callback' in stack_text:
            return 'interpreter_loop (in handle_start_callback)'
        elif 'handle_finish_callback' in stack_text:
            return 'interpreter_loop (in handle_finish_callback)'
        elif 'wait' in stack_text:
            return 'interpreter_loop (waiting)'
        elif 'notify' in stack_text:
            return 'interpreter_loop (notifying)'
        return 'interpreter_loop'
    elif 'callback_loop' in stack_text:
        if 'wait' in stack_text:
            return 'callback_loop (waiting)'
        elif 'notify' in stack_text:
            return 'callback_loop (notifying)'
        return 'callback_loop'
    elif 'pusher_thread_loop_all' in stack_text:
        if 'wait' in stack_text:
            return 'server_pusher (waiting)'
        return 'server_pusher'
    elif 'pusher_thread_loop' in stack_text:
        if 'wait' in stack_text:
            return 'client_pusher (waiting)'
        return 'client_pusher'
    elif 'uv_run' in stack_text:
        return 'uv_loop'
    elif 'TestBody' in stack_text or 'main' in stack_text:
        return 'main_thread'
    return 'unknown'


def find_interesting_frame(frames, thread_type):
    """Find the frame index with the interesting function for this thread type."""
    for i, frame in enumerate(frames):
        func = frame.get('func', '')
        if 'interpreter_loop' in thread_type and func == 'interpreter_loop':
            return i
        elif 'callback_loop' in thread_type and func == 'callback_loop':
            return i
        elif 'pusher' in thread_type and 'pusher_thread_loop' in func:
            return i
    return None


def get_thread_info_with_gdbmi(pid):
    """Use pygdbmi to get detailed thread information."""
    threads_info = {}
    
    try:
        # Create GDB controller and attach to process
        gdbmi = GdbController()
        
        # Attach to the process
        response = gdbmi.write(f"-target-attach {pid}", timeout_sec=10)
        
        # Get thread list
        response = gdbmi.write("-thread-info", timeout_sec=10)
        
        threads = []
        for r in response:
            if r.get('type') == 'result' and r.get('payload'):
                threads = r['payload'].get('threads', [])
                break
        
        print(f"\n=== Found {len(threads)} threads ===")
        
        for thread in threads:
            thread_id = thread.get('id')
            target_id = thread.get('target-id', '')
            state = thread.get('state', '')
            
            # Switch to this thread
            gdbmi.write(f"-thread-select {thread_id}", timeout_sec=5)
            
            # Get backtrace for this thread
            bt_response = gdbmi.write("-stack-list-frames", timeout_sec=5)
            
            frames = []
            for r in bt_response:
                if r.get('type') == 'result' and r.get('payload'):
                    frames = r['payload'].get('stack', [])
                    break
            
            # Parse frames into a more usable format
            parsed_frames = []
            for frame in frames:
                if isinstance(frame, dict):
                    parsed_frames.append(frame)
                elif isinstance(frame, tuple) and len(frame) == 2:
                    # Sometimes frames come as ('frame', {...})
                    parsed_frames.append(frame[1])
            
            thread_type = identify_thread_type(parsed_frames)
            
            threads_info[int(thread_id)] = {
                'target_id': target_id,
                'state': state,
                'frames': parsed_frames,
                'type': thread_type
            }
        
        # Print thread summary
        print("\n=== Thread Summary ===")
        for thread_id, info in sorted(threads_info.items()):
            print(f"Thread {thread_id}: {info['type']} ({info['state']})")
        
        # Get detailed info from interesting threads
        print("\n=== Detailed Thread State ===")
        for thread_id, info in sorted(threads_info.items()):
            thread_type = info['type']
            frames = info['frames']
            
            frame_idx = find_interesting_frame(frames, thread_type)
            
            print(f"\n--- Thread {thread_id} ({thread_type}) ---")
            
            # Switch to thread
            gdbmi.write(f"-thread-select {thread_id}", timeout_sec=5)
            
            if frame_idx is not None:
                gdbmi.write(f"-stack-select-frame {frame_idx}", timeout_sec=5)
                
                # Get local variables
                locals_response = gdbmi.write("-stack-list-locals 1", timeout_sec=5)
                
                for r in locals_response:
                    if r.get('type') == 'result' and r.get('payload'):
                        locals_list = r['payload'].get('locals', [])
                        print("Local variables:")
                        for local in locals_list:
                            if isinstance(local, dict):
                                name = local.get('name', '?')
                                value = local.get('value', '?')
                                print(f"  {name} = {value}")
                        break
            
            # Print the stack trace for this thread
            print("Stack trace:")
            for i, frame in enumerate(frames[:15]):  # Limit to first 15 frames
                func = frame.get('func', '??')
                file = frame.get('file', '??')
                line = frame.get('line', '?')
                marker = " <-- " if i == frame_idx else ""
                print(f"  #{i} {func} at {file}:{line}{marker}")
        
        # Detach and exit
        gdbmi.write("-target-detach", timeout_sec=5)
        gdbmi.exit()
        
    except Exception as e:
        print(f"Error using pygdbmi: {e}")
        import traceback
        traceback.print_exc()
    
    return threads_info


def run_single_test(run_id):
    """Run a single test instance and return the result."""
    if failure_found.is_set():
        return {"run": run_id, "status": "skipped"}
    
    # Create output files for this run
    stdout_file = os.path.join(OUTPUT_DIR, f"run_{run_id}_stdout.txt")
    stderr_file = os.path.join(OUTPUT_DIR, f"run_{run_id}_stderr.txt")
    strace_file = os.path.join(OUTPUT_DIR, f"run_{run_id}_strace.txt")
    
    with open(stdout_file, 'w') as stdout_f, open(stderr_file, 'w') as stderr_f:
        proc = subprocess.Popen(
            [
              #'strace', '-f', '-o', strace_file,
              TEST_PATH
            ],
            stdout=stdout_f,
            stderr=stderr_f
        )
        
        try:
            proc.wait(timeout=TIMEOUT)
            
            # Read the output file to check for PASSED
            with open(stdout_file, 'r') as f:
                stdout_text = f.read()
            
            if "PASSED" in stdout_text:
                # Clean up successful run files
                os.remove(stdout_file)
                os.remove(stderr_file)
                return {"run": run_id, "status": "passed"}
            else:
                return {
                    "run": run_id,
                    "status": "failed",
                    "exit_code": proc.returncode,
                    "stdout_file": stdout_file,
                    "stderr_file": stderr_file
                }
                
        except subprocess.TimeoutExpired:
            return {
                "run": run_id,
                "status": "timeout",
                "proc": proc,
                "stdout_file": stdout_file,
                "stderr_file": stderr_file
            }


def handle_timeout(run_id, proc, stdout_file, stderr_file):
    """Handle a timeout by getting GDB backtraces using pygdbmi."""
    print(f"\n=== TIMEOUT at run {run_id} ===")
    print(f"Output files: {stdout_file}, {stderr_file}")
    
    # Check if process is still running before trying to attach GDB
    poll_result = proc.poll()
    if poll_result is not None:
        print(f"Process already exited with code {poll_result}")
        print(f"Output files available at: {stdout_file}, {stderr_file}")
        return
    
    print(f"Process {proc.pid} is still running...")
    
    print("\n=== Getting GDB backtraces using pygdbmi ===")
    
    try:
        get_thread_info_with_gdbmi(proc.pid)
    except Exception as e:
        print(f"Error getting GDB info: {e}")
        import traceback
        traceback.print_exc()
    
    # Kill the hung process
    try:
        proc.kill()
        proc.wait(timeout=5)
    except Exception as e:
        print(f"Error killing process: {e}")
    
    print(f"Output files available at: {stdout_file}, {stderr_file}")


def print_output_location(stdout_file, stderr_file):
    """Print the location of output files for inspection."""
    print(f"  stdout: {stdout_file}")
    print(f"  stderr: {stderr_file}")


def handle_failure(run_id, exit_code, stdout_file, stderr_file):
    """Handle a test failure."""
    print(f"\n=== FAILED at run {run_id} (exit code {exit_code}) ===")
    print(f"Output files available at:")
    print_output_location(stdout_file, stderr_file)


# Main execution loop with parallel threads
print(f"Running tests with {PARALLEL_THREADS} parallel threads...")
completed_runs = 0
failed = False

with ThreadPoolExecutor(max_workers=PARALLEL_THREADS) as executor:
    # Submit all runs
    futures = {executor.submit(run_single_test, i): i for i in range(1, MAX_RUNS + 1)}
    
    for future in as_completed(futures):
        run_id = futures[future]
        try:
            result = future.result()
            
            if result["status"] == "passed":
                completed_runs += 1
                # Print progress every 10 runs
                if completed_runs % 10 == 0:
                    print(f"Completed {completed_runs} runs...")
            elif result["status"] == "skipped":
                pass  # Another thread already found a failure
            elif result["status"] == "failed":
                handle_failure(result["run"], result["exit_code"], result["stdout_file"], result["stderr_file"])
            elif result["status"] == "timeout":
                if not failure_found.is_set():
                    failure_found.set()
                    handle_timeout(result["run"], result["proc"], result["stdout_file"], result["stderr_file"])
                    failed = True
                else:
                    # Kill the process if another failure was already found
                    try:
                        result["proc"].kill()
                        result["proc"].wait()
                    except:
                        pass
        except Exception as e:
            print(f"Error in run {run_id}: {e}")
            if not failure_found.is_set():
                failure_found.set()
                failed = True

if not failed:
    print(f"\nAll {MAX_RUNS} runs passed!")

print(f"Completed after {completed_runs} successful runs")
