#!/usr/bin/python3

import eth
from eth_abi import decode
import aiohttp
import requests
import websockets
import asyncio
import json

keys = json.load(open('keys.json', 'r'))
factories = json.load(open('factories.json', 'r'))
routers = json.load(open('routers.json', 'r'))
infura_url = f"https://mainnet.infura.io/v3/{keys['INFURA']}"
infura_ws_url = f"wss://mainnet.infura.io/ws/v3/{keys['INFURA']}"


# Known swap function selectors (Uniswap V2, V3, etc.)
swap_function_selectors = {
    "0x38ed1739": "Uniswap V2: swapExactTokensForTokens",
    "0x8803dbee": "Uniswap V2: swapTokensForExactTokens",
    "0x7ff36ab5": "Uniswap V2: swapExactETHForTokens",
    "0x4a25d94a": "Uniswap V2: swapTokensForExactETH",
    "0x18cbafe5": "Uniswap V2: swapExactTokensForETH",
    "0xfb3bdb41": "Uniswap V2: swapETHForExactTokens",
    "0x414bf389": "Uniswap V3: exactInputSingle",
    "0x472b43f3": "Uniswap V3: exactInput",
    "0x3593564c": "Uniswap V3: exactOutputSingle",
    "0x09abca6a": "Uniswap V3: exactOutput",
}

async def fetch_transaction(session, txn_hash):
    rpc_payload = {
        "jsonrpc": "2.0",
        "method": "eth_getTransactionByHash",
        "params": [txn_hash],
        "id": 1,
    }
    try:
        async with session.post(infura_url,json=rpc_payload) as response:
            if response.status == 200:
                result = await response.json()
                return result.get("result", None)
    except Exception as e:
        print(f"Error fetching transaction {txn_hash}: {e}")
    return None

async def process_transaction(txn_hash, session):
    txn = await fetch_transaction(session, txn_hash)
    if txn:
        input_data = txn.get("input", "")
        if input_data:
            function_selector = input_data[:10]  # First 4 bytes
            if function_selector in swap_function_selectors:
                print(f"Swap detected: {swap_function_selectors[function_selector]}")
                print(f"Transaction Hash: {txn_hash}")
                print(f"From: {txn['from']}")
                print(f"To: {txn['to']}")
                print(f"Value (in Wei): {txn['value']}")
                try:
                    if function_selector == "0x38ed1739":  # swapExactTokensForTokens
                        # Decode params: address[] path, uint amountOutMin, address to, uint deadline
                        params = decode(["uint256", "uint256", "address[]", "address", "uint256"], bytes.fromhex(input_data[10:]))
                        path = params[2]  # path is an array of token addresses
                        print(f"Swapping: {' -> '.join(path)}")
                    elif function_selector == "0x7ff36ab5":  # swapExactETHForTokens
                        # Decode params: uint amountOutMin, address[] path, address to, uint deadline
                        params = decode(["uint256", "address[]", "address", "uint256"], bytes.fromhex(input_data[10:]))
                        path = params[1]
                        symbols = [token_addresses.get(addr.lower(), addr) for addr in path]
                        print(f"Swapping: {' -> '.join(path)}")
                except Exception as e:
                    print(f"Error decoding input: {e}")

async def listen_pending_transactions():
    async with websockets.connect(infura_ws_url) as ws, aiohttp.ClientSession() as session:
        # Subscribe to newPendingTransactions
        subscription_payload = {
            "jsonrpc": "2.0",
            "method": "eth_subscribe",
            "params": ["newPendingTransactions"],
            "id": 1,
        }
        await ws.send(json.dumps(subscription_payload))

        while True:
            try:
                message = await ws.recv()
                response = json.loads(message)
                if "params" in response:
                    txn_hash = response["params"]["result"]
                    asyncio.create_task(process_transaction(txn_hash, session))
            except Exception as e:
                print(f"Error: {e}")
                break

if __name__ == "__main__":
    asyncio.run(listen_pending_transactions())
