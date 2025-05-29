import random
import string
import threading
from concurrent.futures import ThreadPoolExecutor
import time

def setup():
    flag = 'THM{thisisafakeflag}'
    xored = ""
    res = ''.join(random.choices(string.ascii_letters + string.digits, k=5))
    key = str(res)

    for i in range(0, len(flag)):
        xored += chr(ord(flag[i]) ^ ord(key[i % len(key)]))

    hex_encoded = xored.encode().hex()
    return hex_encoded

target = "373a19293e5213383c3a260a20133a174637392d221c26612f0f3e2d3a1b11"

def crack_attempt(thread_id, attempts_per_thread):
    start_time = time.time()
    for i in range(attempts_per_thread):
        result = setup()
        if result == target:
            end_time = time.time()
            print(f'Success!! Key found by thread {thread_id}')
            print(f'Result: {result}')
            print(f'Time taken: {end_time - start_time:.2f} seconds')
            return True
        if i % 1000000 == 0:  # Progress update every 10000 attempts
            print(f'Thread {thread_id}: Attempted {i} combinations...')
    return False

def main():
    num_threads = 400  # Adjust based on your CPU cores
    attempts_per_thread = 10000000  # Adjust based on your needs
    
    print(f"Starting brute force with {num_threads} threads...")
    start_time = time.time()
    
    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        futures = [
            executor.submit(crack_attempt, thread_id, attempts_per_thread)
            for thread_id in range(num_threads)
        ]
        
        # Wait for any thread to find the solution or all threads to complete
        for future in futures:
            if future.result():
                executor._threads.clear()
                break
                
    end_time = time.time()
    print(f'Total time elapsed: {end_time - start_time:.2f} seconds')

if __name__ == "__main__":
    main()
