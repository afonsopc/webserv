import sys
import os
import requests
import urllib3
import traceback
import re

urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
PROXY_TARGET = "omelhorsite.pt"


def parse_request(raw_request):
    lines = raw_request.split("\r\n")
    request_line = lines[0].split(" ")
    method = request_line[0] if len(request_line) > 0 else "GET"
    path = request_line[1] if len(request_line) > 1 else "/"
    headers = {}
    i = 1
    while i < len(lines) and lines[i] != "":
        if ":" in lines[i]:
            key, value = lines[i].split(":", 1)
            headers[key.strip()] = value.strip()
        i += 1
    body = "\r\n".join(lines[i + 1 :]) if i + 1 < len(lines) else ""
    return method, path, headers, body


def prepare_headers(original_headers):
    headers = {}
    skip_headers = [
        "host",
        "connection",
        "content-length",
        "transfer-encoding",
        "content-encoding",
        "accept-encoding",
    ]
    for key, value in original_headers.items():
        if key.lower() not in skip_headers:
            headers[key] = value
    headers["Host"] = PROXY_TARGET
    return headers


def rewrite_html_content(content, base_path):
    if not isinstance(content, str):
        try:
            content = content.decode("utf-8")
        except:
            return content
    content = re.sub(rf"https?://{re.escape(PROXY_TARGET)}/", base_path + "/", content)
    content = re.sub(rf"//{re.escape(PROXY_TARGET)}/", base_path + "/", content)
    return content


def send_response_headers(status_code, response_headers, content_length, base_path):
    print(status_code)
    skip_headers = [
        "content-encoding",
        "transfer-encoding",
        "content-length",
        "connection",
        "location",
    ]
    for key, value in response_headers.items():
        if key.lower() not in skip_headers:
            print(f"{key}: {value}")
    if "Location" in response_headers:
        location = response_headers["Location"]
        if location.startswith("http"):
            location = re.sub(
                rf"https?://{re.escape(PROXY_TARGET)}", base_path, location
            )
        print(f"Location: {location}")
    print(f"Content-Length: {content_length}")
    print()


def main():
    try:
        sys.stdout.reconfigure(line_buffering=True)
        if len(sys.argv) < 2:
            print("400")
            print("Content-Type: text/html")
            print()
            print(
                "<html><body><h1>Error</h1><p>No request data received</p></body></html>"
            )
            sys.stdout.flush()
            return
        raw_request = sys.argv[1]
        method, path, headers, body = parse_request(raw_request)
        base_path = os.environ.get("SCRIPT_NAME", "/cgi/proxy")
        if base_path.endswith("/proxy"):
            base_path = base_path[:-6]
        proxy_headers = prepare_headers(headers)
        url = f"https://{PROXY_TARGET}{path}"
        try:
            if method.upper() == "GET":
                response = requests.get(
                    url,
                    headers=proxy_headers,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                    stream=True,
                )
            elif method.upper() == "POST":
                response = requests.post(
                    url,
                    headers=proxy_headers,
                    data=body,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                    stream=True,
                )
            elif method.upper() == "HEAD":
                response = requests.head(
                    url,
                    headers=proxy_headers,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                )
            elif method.upper() == "PUT":
                response = requests.put(
                    url,
                    headers=proxy_headers,
                    data=body,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                    stream=True,
                )
            elif method.upper() == "DELETE":
                response = requests.delete(
                    url,
                    headers=proxy_headers,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                )
            else:
                response = requests.get(
                    url,
                    headers=proxy_headers,
                    verify=False,
                    allow_redirects=False,
                    timeout=5,
                    stream=True,
                )
        except requests.exceptions.Timeout:
            error_msg = f"<html><body><h1>Timeout</h1><p>Request to {url} timed out after 5 seconds</p></body></html>"
            print("504")
            print("Content-Type: text/html")
            print(f"Content-Length: {len(error_msg)}")
            print()
            print(error_msg)
            sys.stdout.flush()
            return
        if response.status_code == 304:
            send_response_headers(response.status_code, response.headers, 0, base_path)
            sys.stdout.flush()
            return
        content = response.content
        content_type = response.headers.get("Content-Type", "").lower()
        if (
            "text/html" in content_type
            or "text/css" in content_type
            or "javascript" in content_type
        ):
            try:
                rewritten = rewrite_html_content(content, base_path)
                if isinstance(rewritten, str):
                    content = rewritten.encode("utf-8")
                else:
                    content = rewritten
            except:
                pass
        send_response_headers(
            response.status_code, response.headers, len(content), base_path
        )
        sys.stdout.flush()
        sys.stdout.buffer.write(content)
        sys.stdout.buffer.flush()

    except Exception as e:
        error_details = traceback.format_exc()
        error_msg = f"<html><body><h1>Proxy Error</h1><p>{str(e)}</p><pre>{error_details}</pre></body></html>"
        print("502")
        print("Content-Type: text/html")
        print(f"Content-Length: {len(error_msg)}")
        print()
        print(error_msg)
        sys.stdout.flush()


if __name__ == "__main__":
    main()
