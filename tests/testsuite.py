import requests
import json
from colorama import Fore, Style

server = 'http://127.0.0.1:8000'
default_headers = {"Host" :"localhost"}

percent = 1
for i in range(1, 101):
    print(percent, "%")
    percent += 1

    r = requests.get(server)


r = requests.get(server, default_headers)

def test_request(r, status_code, test_name):
    if (r.status_code == status_code):
        print(f"{Fore.GREEN}OK{Style.RESET_ALL}")
    else:
        print(f"{Fore.RED}KO: Expected", status_code, "got", r.status_code, f"in{Fore.YELLOW}", test_name, f"{Style.RESET_ALL}")


r = requests.delete(server)
test_request(r, 405, "forbidden use of DELETE")
test_request(r, 200, "test to test if the not working test is working properly")

r = requests.post(server) 
test_request(r, 400, "request without 'host' header")

r = requests.post(server, default_headers)
test_request(r, 200, "post treated as get")

r = requests.get(server, headers={"Host":"localhost","Content-Length":"10"})
test_request(r, 400, "get with content length")

# r = requests.post(server, headers={"Host":"localhost","Content-Length":"10"})
# test_request(r, 400, "post with content length")

r = requests.post(server, headers={"Host":"localhost","Content-Length":"10"}, data="0")
test_request(r, 200, "post with body")

print(r.text)
print(r.status_code)