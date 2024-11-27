#!/usr/bin/python3

import eth
import asyncio
import websockets
import json

keys = json.load(open('keys.json','r'))
exchanges = json.load(open('exchanges.json','r'))
infura_url = f"https://mainnet.infura.io/v3/{keys['INFURA']}"
infura_ws_url = f"wss://mainnet.infura.io/ws/v3/{keys['INFURA']}"

async def subscribe_to_new_heads():
# JSON-RPC subscription payload for "newHeads"
    subscription_payload = {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "eth_subscribe",
        "params": ["newHeads"]
    }
    async with websockets.connect(infura_ws_url) as websocket:
        # Send subscription request
        await websocket.send(json.dumps(subscription_payload))
        print("Subscription request sent...")

        # Await subscription confirmation
        response = await websocket.recv()
        print("Subscription confirmed:", response)

        # Listen for new block headers
        print("Listening for new block headers...")
        while True:
            try:
                message = await websocket.recv()
                data = eth.decode_block_data(json.loads(message))
                block = eth.get_block_transactions(infura_url,data['number'])
                for tx in block['transactions']:
                    tx_receipt = eth.get_transaction_receipt(infura_url,tx['hash'])
                    for log in tx_receipt['logs']:
                        for topic in log['topics']:
                            is_swap = eth.is_swap(topic)
                            if is_swap == 1 or is_swap == 2:
                                print(json.dumps(tx_receipt,indent=4))
                                tokens = eth.get_swap_token_pair(infura_url, log['address'])
                                print(tokens)
                                for exchange in exchanges:
                                    print(exchange)
                                    is_valid_pair = eth.get_pair_address(infura_url,exchanges[exchange]['factory'],tokens[0],tokens[1])
                                    if is_valid_pair:
                                        decimalA = eth.get_token_decimals(infura_url,tokens[0])
                                        decimalB = eth.get_token_decimals(infura_url,tokens[1])
                                        reserveA, reserveB, ts = eth.get_pair_reserves(infura_url,is_valid_pair)
                                        print(decimalA,decimalB)
                                        print(ts,reserveA,reserveB)
                                        price_of_A_in_B = (reserveB / 10**decimalB) / (reserveA / 10**decimalA)
                                        print(price_of_A_in_B)
                                    print(exchange,is_valid_pair)
                print("New block data received:")
                print(json.dumps(data, indent=4))
            except Exception as e:
                print(f"Error receiving message: {e}")
                break

# Run the script
if __name__ == "__main__":
    asyncio.run(subscribe_to_new_heads())

