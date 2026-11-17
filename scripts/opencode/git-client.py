#!/usr/bin/python3
import json,socket,sys
if len(sys.argv)<2:raise SystemExit(64)
raw=json.dumps({"git":sys.argv[1:]}).encode()
if len(raw)>16000:raise SystemExit(64)
with socket.socket(socket.AF_UNIX,socket.SOCK_STREAM) as s:
 s.settimeout(110);s.connect("/run/nexvary-mr-check/control.sock");s.sendall(raw);s.shutdown(socket.SHUT_WR)
 data=bytearray()
 while chunk:=s.recv(65536):
  data.extend(chunk)
  if len(data)>210000:raise SystemExit(65)
code,_,body=data.partition(b"\n");sys.stdout.buffer.write(body);raise SystemExit(int(code))
