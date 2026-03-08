
## 🚘 Enable Hands free autopilot
This is a fork of https://github.com/Beat-YT/s3xy-virtual-button
I added the code in main.cpp and platforms.ini to work as follows:

I changed the original code to work with a TTP223B Capacitive Touch Switch but any button would work.
https://www.amazon.com/HiLetgo-TTP223B-Capacitive-Digital-Raspberry/dp/B00HFQEFWQ/

When powered on... The system is OFF
1. Single press of the button calls:  s3xy_send_single();
2. Double press of the button calls:  s3xy_send_double();
3. Long press of the button calls:    s3xy_send_long();
4. 3x press of the button toggle system to start running.

When the systrm is turned on, at random intervals from 9.5 sec to 12.5 sec the system will trigger a single button press.
In the Enhance Auto App I have configures the Single Press to subtract 1 from the current Cuirse Control then add 1 back to the Cruise Control.
This resets the Nag timer.

# ⚠️⚠️ This is never to be used in the real world!! This is just informational only. 
```
# 🟢 S3XY Virtual Button (ESP32)

This is a simple ESP32 sketch that turns your dev board into a virtual *S3XY Button* — a BLE accessory that mimics Enhance Auto’s real button (`ENH_BTN`) device.

You can:
- Pair it with your Tesla Gen 2 S3XY Commander
- Emulate single click, long press, or double click actions
- Add your own automation logic easily in `main.cpp`

> ⚠️ All BLE, pairing, and hex protocol details are abstracted away. You only deal with simple functions like `s3xy_send_single()` — ideal for beginners or tinkerers.

---

## 📦 Why I Made This

One night I got fed up with the S3XY Commander and expensive buttons.  
I wanted automation. Triggers. Scripts. Control.

Instead, I got a blinking LED and manual presses.

So I reverse-engineered it.

In a day.

This repo is the result.

---

## 🔍 What It Does

The official S3XY Button is a BLE peripheral called `ENH_BTN`.  
When paired with the S3XY Commander (an ESP32-based receiver), it uses encrypted GATT notifications to trigger button actions.  

This project:
- Spoofs the button (same name, same UUIDs)
- Pairs securely with your Commander
- Responds to specific write commands
- Sends the correct BLE notify packets on demand

**Also:**  
The real button adds a ~300 ms delay before processing clicks when multiple actions (like double-click or long-press) are configured — likely to wait and see what you're doing.  
This version sends actions **instantly**. No lag. No guessing.

And yes, it’s fully compatible with the gen2 commander. Tested.

## 🔧 Pairing Procedure (Same as Real Button)

> 🆕 **Note:** The virtual button automatically enters pairing mode on power-up.

To pair it:
1. Power up your ESP32 running this sketch
2. It advertises as `ENH_BTN` and waits for a connection
3. On the S3XY Commander, go to the **Buttons menu → Add S3XY Button**
4. It will detect and bond with your ESP32 automatically
5. Once paired, the Commander will start sending encrypted BLE commands to your virtual button

You can now trigger `s3xy_send_single()` to activate accessories — exactly like the real hardware.

---

## 🔐 Security / Protocol Details

The Commander uses BLE Secure Connections with bonding (just like a real accessory would):
- 16-byte encryption key
- No passkey (Just Works pairing)
- Authentication level: `ESP_LE_AUTH_REQ_SC_BOND`
- Key exchange uses `ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK`

Once bonded, the Commander can securely read/write to the "ID" characteristic and subscribe to button notifications.

> 📝 "ID" isn't some official name — it's just what I called the `0x3d49` characteristic UUID.  
> The Commander writes to it to interact with the button — sending things like handshake checks, rename commands, and disconnect requests.

This sketch emulates the real button behavior:
- When the Commander writes `0xB6`, it replies with `C7 00 01` (firmware/version check?)
- When it writes `0xA1`, we disconnect (unpair)
- When it writes `A4 XX YY ZZ`, we send a rename response (used for S - E - X - Y button tags?)

All UUIDs and byte patterns were captured and decoded from real ENH_BTN ↔ Commander traffic.

> 🧪 Sniffing BLE traffic was done with nRF Connect + a rooted Android phone + wireshark
