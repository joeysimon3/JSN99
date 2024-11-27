#!/usr/bin/python3

import binascii
import requests
import json
from eth_utils import keccak, to_bytes
from hashlib import sha3_256

def is_swap(log_topic):
    try:
        v2_pool_swap_sig = "Swap(address,uint256,uint256,uint256,uint256,address)"
        v3_pool_swap_sig = "Swap(address,address,int256,int256,uint160,uint128,int24)"
        v2_pool_swap_sig_bytes = keccak(text=v2_pool_swap_sig)
        v3_pool_swap_sig_bytes = keccak(text=v3_pool_swap_sig)
        if log_topic == '0x'+v2_pool_swap_sig_bytes.hex(): return 1
        if log_topic == '0x'+v3_pool_swap_sig_bytes.hex(): return 2
        return 0
    except Exception as e:
        print('is_swap',str(e))

def is_swap_v2(input_log):
    try:
        print(input_log[:10])
        if input_log[:10] in ['0x38ed1739','0x8803dbee','0x7ff36ab5','0x4a25d94a','0x18cbafe5','0xfb3bdb41']:
            return 1
        else:
            return 0
    except Exception as e:
        print(str(e))

def decode_block_data(data):
    try:
        """
        Decodes an Ethereum block data object.
        Converts hex values to integers or strings where applicable.
        """
        result = data.get("params", {}).get("result", {})
        decoded = {}

        # Decode block details
        decoded["subscription"] = data.get("params", {}).get("subscription", None)
        decoded["baseFeePerGas"] = int(result.get("baseFeePerGas", "0x0"), 16)
        decoded["blobGasUsed"] = int(result.get("blobGasUsed", "0x0"), 16)
        decoded["difficulty"] = int(result.get("difficulty", "0x0"), 16)
        decoded["excessBlobGas"] = int(result.get("excessBlobGas", "0x0"), 16)
        decoded["extraData"] = binascii.unhexlify(result.get("extraData", "0x")[2:]).decode("utf-8", errors="ignore")
        decoded["gasLimit"] = int(result.get("gasLimit", "0x0"), 16)
        decoded["gasUsed"] = int(result.get("gasUsed", "0x0"), 16)
        decoded["hash"] = result.get("hash")
        decoded["logsBloom"] = result.get("logsBloom")
        decoded["miner"] = result.get("miner")
        decoded["mixHash"] = result.get("mixHash")
        decoded["nonce"] = result.get("nonce")
        decoded["number"] = int(result.get("number", "0x0"), 16)
        decoded["parentBeaconBlockRoot"] = result.get("parentBeaconBlockRoot")
        decoded["parentHash"] = result.get("parentHash")
        decoded["receiptsRoot"] = result.get("receiptsRoot")
        decoded["sha3Uncles"] = result.get("sha3Uncles")
        decoded["stateRoot"] = result.get("stateRoot")
        decoded["timestamp"] = int(result.get("timestamp", "0x0"), 16)
        decoded["totalDifficulty"] = int(result.get("totalDifficulty", "0x0"), 16)
        decoded["transactionsRoot"] = result.get("transactionsRoot")
        decoded["withdrawalsRoot"] = result.get("withdrawalsRoot")

        return decoded
    except Exception as e:
        print('decode',str(e))

def get_block_transactions(INFURA_URL,block_number):
    try:
        """
        Fetch transactions from a block by block number.
        """
        # Create JSON-RPC payload
        payload = {
            "jsonrpc": "2.0",
            "method": "eth_getBlockByNumber",
            "params": [hex(block_number), True],  # True to include full transaction objects
            "id": 1
        }

        # Make the request
        response = requests.post(INFURA_URL, json=payload)
        if response.status_code != 200:
            raise Exception(f"Request failed with status code {response.status_code}: {response.text}")

        # Parse the JSON response
        block_data = response.json()

        if "error" in block_data:
            raise Exception(f"Error in response: {block_data['error']}")

        return block_data["result"]
    except Exception as e:
        print('transactions',str(e))

def get_transaction_receipt(INFURA_URL,tx_hash):
    try:
        """
        Fetch transactions from a block by block number.
        """
        # Create JSON-RPC payload
        payload = {
            "jsonrpc": "2.0",
            "method": "eth_getTransactionReceipt",
            "params": [tx_hash],  # True to include full transaction objects
            "id": 1
        }

        # Make the request
        response = requests.post(INFURA_URL, json=payload)
        if response.status_code != 200:
            raise Exception(f"Request failed with status code {response.status_code}: {response.text}")

        # Parse the JSON response
        tx_receipt = response.json()

        if "error" in tx_receipt:
            raise Exception(f"Error in response: {tx_receipt['error']}")

        return tx_receipt['result']
    except Exception as e:
        print('receipt',str(e))

def get_swap_token_pair(INFURA_URL, contract_address):
    try: 
        """
        Fetch the token pair (token0 and token1) from a Uniswap-like contract.
        """
        def call_contract_function(method_sig):
            payload = {
                "jsonrpc": "2.0",
                "method": "eth_call",
                "params": [
                    {
                        "to": contract_address,
                        "data": "0x" + keccak(text=method_sig).hex()[:8]  # First 4 bytes of the method signature
                    },
                    "latest"
                ],
                "id": 1
            }
            response = requests.post(INFURA_URL, json=payload)
            if response.status_code != 200:
                raise Exception(f"Request failed with status code {response.status_code}: {response.text}")
            result = response.json()
            if "error" in result:
                raise Exception(f"Error in response: {result['error']}")
            return "0x" + result["result"][26:]  # Decode the returned address

        token0 = call_contract_function("token0()")
        token1 = call_contract_function("token1()")
        return token0, token1
    except Exception as e:
        print('pair',str(e))

def get_pair_address(INFURA_URL, factory_address, tokenA_address, tokenB_address):
    try: 
        """
        Fetch the pair address for two tokens from a Uniswap-like factory contract using the Infura API.
        """
        def call_contract_function(method_sig, tokenA, tokenB):
            # Hash the method signature to get the first 4 bytes of its Keccak-256 hash
            method_id = keccak(text=method_sig).hex()[:8]
            # Prepare the data payload with zero-padded token addresses
            data = "0x" + method_id + tokenA[2:].rjust(64, '0') + tokenB[2:].rjust(64, '0')

            payload = {
                "jsonrpc": "2.0",
                "method": "eth_call",
                "params": [
                    {
                        "to": factory_address,
                        "data": data
                    },
                    "latest"
                ],
                "id": 1
            }
            response = requests.post(INFURA_URL, json=payload)
            if response.status_code != 200:
                raise Exception(f"Request failed with status code {response.status_code}: {response.text}")

            result = response.json()
            if "error" in result:
                raise Exception(f"Error in response: {result['error']}")

            # Decode the returned address, stripping leading zeros
            return "0x" + result["result"][26:]

        # Get the pair address using the "getPair(address,address)" function signature
        pair_address = call_contract_function("getPair(address,address)", tokenA_address, tokenB_address)

        # Output and return results based on the pair address
        if pair_address != "0x0000000000000000000000000000000000000000":
            print(f"Pair exists: {pair_address}")
        else:
            print("Pair does not exist.")
            return None
        return pair_address
    except Exception as e:
        print('address',str(e))

def get_token_decimals(INFURA_URL, token_address):
    try:
        """
        Fetch the pair address for two tokens from a Uniswap-like factory contract using the Infura API.
        """
        def call_contract_function(method_sig):

            payload = {
                "jsonrpc": "2.0",
                "method": "eth_call",
                "params": [
                    {
                        "to": token_address,
                        "data": "0x" + keccak(text=method_sig).hex()[:8]  # First 4 bytes of the method signature
                    },
                    "latest"
                ],
                "id": 1
            }
            response = requests.post(INFURA_URL, json=payload)
            if response.status_code != 200:
                raise Exception(f"Request failed with status code {response.status_code}: {response.text}")

            result = response.json()
            if "error" in result:
                raise Exception(f"Error in response: {result['error']}")

            # Decode the returned address, stripping leading zeros
            return "0x" + result["result"][26:]

        # Get the pair address using the "getPair(address,address)" function signature
        decimals = call_contract_function("decimals()")

        return int(decimals, 16)
    except Exception as e:
        print('decimals',str(e))

def get_pair_reserves(INFURA_URL, contract_address):
    try:
        """
        Fetch the token pair reserves from a Uniswap-like contract.
        """
        def call_contract_function(method_sig):
            # Hash the method signature to get the first 4 bytes (8 hex characters)
            payload = {
                "jsonrpc": "2.0",
                "method": "eth_call",
                "params": [
                    {
                        "to": contract_address,
                        "data": "0x" + keccak(text=method_sig).hex()[:8]  # First 4 bytes of the method signature
                    },
                    "latest"
                ],
                "id": 1
            }
            response = requests.post(INFURA_URL, json=payload)
            if response.status_code != 200:
                raise Exception(f"Request failed with status code {response.status_code}: {response.text}")
            result = response.json()
            if "error" in result:
                raise Exception(f"Error in response: {result['error']}")
            # Correctly extract the reserve values (taking into account the length of uint112 and uint32)
            print(result)
            data = result["result"]
            data = data[2:]
            reserve0 = int(data[:64],16)
            reserve1 = int(data[64:128],16)
            ts = int(data[128:],16)
            return reserve0, reserve1, ts

        reserves = call_contract_function("getReserves()")
        return reserves
    except Exception as e:
        print('Error fetching reserves:', str(e))

