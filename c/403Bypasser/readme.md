📦 Compilation & Installation
bash


# Install dependencies (Ubuntu/Debian)
sudo apt-get install libcurl4-openssl-dev build-essential
# Install dependencies (Fedora/RHEL)
sudo dnf install libcurl-devel gcc
# Install dependencies (macOS)
brew install curl
# Compile the tool
gcc -o 403bypass 403bypass.c -lcurl -lpthread -Wall -O2
# Make it executable
chmod +x 403bypass
# Optional: Install system-wide
sudo mv 403bypass /usr/local/bin/
🎯 Usage Examples
1. Basic Scan
bash


./403bypass -u https://example.com/admin
2. With SOCKS5 Proxy (Tor)
bash


./403bypass -u https://example.com/admin -p socks5://127.0.0.1:9050
3. With HTTP Proxy
bash


./403bypass -u https://example.com/admin -p http://proxy.example.com:8080
4. Rate Limited Scan (5 requests per second)
bash


./403bypass -u https://example.com/admin -r 5
5. Multi-threaded with Rate Limiting
bash


./403bypass -u https://example.com/admin -t 3 -r 10
6. With Cookies
bash


./403bypass -u https://example.com/admin -c cookies.txt
7. JSON Output to File
bash


./403bypass -u https://example.com/admin -j -o results.json
8. Full Featured Scan
bash


./403bypass -u https://example.com/admin \
  -p socks5://127.0.0.1:9050 \
  -c cookies.txt \
  -r 10 \
  -t 5 \
  -j \
  -o output.json \
  -v
9. Verbose Mode
bash


./403bypass -u https://example.com/admin -v
10. Help Menu
bash


./403bypass --help
🔧 Features Breakdown
✅ Proxy Support
SOCKS5: socks5://127.0.0.1:9050
SOCKS4: socks4://127.0.0.1:1080
HTTP/HTTPS: http://proxy.example.com:8080
✅ Rate Limiting
Prevents overwhelming the target
Configurable requests per second
Uses microsecond precision timing
✅ Multi-threading (Framework Ready)
Thread pool architecture
Configurable thread count (1-10)
Thread-safe output with mutexes
✅ Cookie Jar
Session persistence
Automatic cookie handling
Compatible with Netscape cookie format
✅ JSON Output
Machine-readable format
Timestamp included
Structured results array
✅ Professional Help Menu
Detailed usage instructions
Multiple examples
Color-coded for readability
📊 Sample Output
Normal Output:


╔════════════════════════════════════════════════════════════════════════════╗
║                   403 Bypass Testing Tool v2.0                          ║
║                     Educational Purpose Only                               ║
║                  Use Only on Authorized Systems                            ║
╚════════════════════════════════════════════════════════════════════════════╝
⚠️  WARNING: Use only on systems you own or have permission to test!
CONFIGURATION:
  Target URL:    https://example.com/admin
  Proxy:         socks5://127.0.0.1:9050 (socks5)
  Rate Limit:    10 req/s
  Threads:       3
[*] Baseline: Status=403 | Size=1234 | Time=0.45s
════════════════════════════════════════════════════════════════════════════
[*] Testing Header Manipulations
════════════════════════════════════════════════════════════════════════════
[403] Header: X-Original-URL                      | Size: 1234   | Time: 0.23s
[200] Header: X-Forwarded-For                     | Size: 5678   | Time: 0.34s | ✓ BYPASS!
[403] Header: X-Rewrite-URL                       | Size: 1234   | Time: 0.28s
...
JSON Output:
json


{
  "target": "https://example.com/admin",
  "timestamp": 1704067200,
  "results": [
    {
      "method": "Header: X-Original-URL",
      "status_code": 403,
      "response_size": 1234,
      "time_taken": 0.234,
      "notes": "/admin"
    },
    {
      "method": "Header: X-Forwarded-For",
      "status_code": 200,
      "response_size": 5678,
      "time_taken": 0.345,
      "notes": "127.0.0.1"
    }
  ]
}
🛡️ Security & Legal
Ethical Use Guidelines:
✅ Test only your own systems
✅ Get written permission for penetration tests
✅ Use in controlled lab environments
❌ Never test without authorization
❌ Don't use for malicious purposes
Defensive Recommendations:
Implement proper authentication/authorization
Normalize all paths before access checks
Don't trust client-supplied headers
Use a Web Application Firewall (WAF)
Regular security audits
🚀 Future Enhancements
You can extend this tool with:

Custom payload injection
Response diff analysis
Automatic success detection
Integration with Burp Suite
Database result storage
Fuzzing capabilities