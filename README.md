## How to Use the Autonomous Surveillance Bot Code

### Prerequisites

- ESP32-CAM development board (AI Thinker or compatible)
- PIR motion sensor connected to GPIO 13
- Stable WiFi network
- Telegram bot token and chat ID (for alerts)

### Hardware Connections

- Connect the PIR sensor’s output pin to GPIO 13 on the ESP32-CAM board.
- Connect the ESP32-CAM pins as per the AI Thinker module pin configuration detailed in the code.
- Power the ESP32-CAM with a suitable 5V source.

### Software Setup

1. **Configure WiFi Credentials**  
   Open the `.ino` file and set your WiFi SSID and password:  
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

2. **Telegram Bot Setup**  
- Create a Telegram bot via [BotFather](https://t.me/botfather) and get your bot token.  
- Retrieve your personal chat ID (you can use a bot like [userinfobot](https://t.me/useridbot) to get your chat ID).  
- Edit the code and replace `"YOUR_BOT_TOKEN"` and `"YOUR_CHAT_ID"` with your values.

3. **Upload Firmware**  
- Open the Arduino IDE.  
- Select the ESP32 board and the right COM port.  
- Build and upload the `.ino` sketch to your ESP32-CAM board.

4. **Run & Monitor**  
- Open Serial Monitor (115200 baud) to view logs.  
- When motion is detected, the ESP32 captures an image and sends it to your Telegram.  
- View the live stream at the ESP32’s local IP printed on the Serial Monitor.

### Notes and Tips

- Adjust the motion detection debounce delay (`delay(500)`) and cooldown (`delay(10000)`) as needed.  
- Ensure your network firewall allows outbound HTTPS for Telegram API calls.  
- Secure your Telegram bot token and chat ID to prevent misuse.
