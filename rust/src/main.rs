use web3::types::{H256, Bytes};
use web3::Web3;
use web3::transports::WebSocket;
use tokio_stream::StreamExt; // Import StreamExt for using `next()`
use std::collections::HashMap;

#[tokio::main]
async fn main() -> web3::Result<()> {
    let ws = WebSocket::new("wss://mainnet.infura.io/ws/v3/1c9cc9b8f6854f7cb55e578fff64f189").await?;
    let web3 = Web3::new(ws);

    let mut sub = web3.eth_subscribe().subscribe_new_pending_transactions().await?;
    println!("Listening for pending transactions...");

    while let Some(Ok(tx_hash)) = sub.next().await {
        handle_pending_transaction(&web3, tx_hash).await?;
    }

    tokio::signal::ctrl_c().await.expect("failed to listen for event");
    Ok(())
}

async fn handle_pending_transaction(web3: &Web3<WebSocket>, tx_hash: H256) -> web3::Result<()> {
    if let Some(tx) = web3.eth().transaction(tx_hash.into()).await? {
        // No need for Option checking on `input` since it's `Bytes`
        let input = &tx.input;

        if input.0.len() >= 4 {
            let function_selector = &input.0[..4]; // Get the first 4 bytes
            let swap_function_selectors = swap_selectors();

            if let Some(description) = swap_function_selectors.get(function_selector) {
                println!("Swap detected: {}", description);
                println!("Transaction Hash: {:?}", tx.hash);
                println!("From: {:?}", tx.from);
                println!("To: {:?}", tx.to);
                println!("Value (in Wei): {:?}", tx.value);
            }
        }
    }
    Ok(())
}

fn swap_selectors() -> HashMap<[u8; 4], &'static str> {
    let mut map = HashMap::new();
    map.insert([0x38, 0xed, 0x17, 0x39], "Uniswap V2: swapExactTokensForTokens");
    map.insert([0x88, 0x03, 0xdb, 0xee], "Uniswap V2: swapTokensForExactTokens");
    map.insert([0x7f, 0xf3, 0x6a, 0xb5], "Uniswap V2: swapExactETHForTokens");
    map.insert([0x4a, 0x25, 0xd9, 0x4a], "Uniswap V2: swapTokensForExactETH");
    map.insert([0x18, 0xcb, 0xaf, 0xe5], "Uniswap V2: swapExactTokensForETH");
    map.insert([0xfb, 0x3b, 0xdb, 0x41], "Uniswap V2: swapETHForExactTokens");
    map.insert([0x41, 0x4b, 0xf3, 0x89], "Uniswap V3: exactInputSingle");
    map.insert([0x47, 0x2b, 0x43, 0xf3], "Uniswap V3: exactInput");
    map.insert([0x35, 0x93, 0x56, 0x4c], "Uniswap V3: exactOutputSingle");
    map.insert([0x09, 0xab, 0xca, 0x6a], "Uniswap V3: exactOutput");
    map
}

