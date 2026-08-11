import socket
import struct
import time
import random

# --- Configuration ---
HOST = '127.0.0.1'
PORT = 8080
# Protocol Format: Little Endian (<), 8-byte Char(8s), Double(d), Uint32(I), Uint64(Q)
# Matches C++ struct alignment
TICK_FORMAT = "<8sdIQ"

def create_packet(symbol_str, price, volume):
    """Encodes market data into a binary UDP packet."""
    # 1. Pad symbol to 8 bytes (null-terminated)
    symbol_bytes = symbol_str.encode('utf-8').ljust(8, b'\0')
    
    # 2. Get high-precision timestamp (microseconds)
    timestamp = int(time.time() * 1000000)
    
    # 3. Pack data into binary format
    return struct.pack(TICK_FORMAT, symbol_bytes, price, volume, timestamp)

def run_simulator():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = (HOST, PORT)

    print(f"Starting HFT Simulator targeting {HOST}:{PORT}...")
    print("Press Ctrl+C to stop.")

    try:
        tick_count = 0
        # Initial prices
        stocks = {"AAPL": 150.00, "GOOG": 2800.00}

        while True:
            # Select random ticker
            symbol = random.choice(list(stocks.keys()))
            
            # Simulate Random Walk (Price fluctuation)
            stocks[symbol] += random.uniform(-1.0, 1.0)
            price = stocks[symbol]
        
            # Serialize and Broadcast
            packet = create_packet(symbol, price, 100)
            sock.sendto(packet, server_address)
            
            # Throttle speed (approx. 100 ticks/sec)
            time.sleep(0.01) 

            # Flash Crash Simulation Logic (Every 100 ticks)
            tick_count += 1
            if tick_count % 100 == 0:
                 crash_symbol = random.choice(list(stocks.keys()))
                 print(f"\n[EVENT] Triggering Flash Crash on {crash_symbol} (-$20.00)")
                 stocks[crash_symbol] -= 20.00
                 # Prevent negative prices
                 if stocks[crash_symbol] < 10.0:
                    stocks[crash_symbol] = 10.0
            
    except KeyboardInterrupt:
        print("\nSimulator stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    run_simulator()