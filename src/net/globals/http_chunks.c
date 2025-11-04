static struct {
  str space;
  str colon;
  str crlf;
  str terminator;
  str version_1;
  str version_1_1;
  str version_2;
  str version_3;
} http_chunks = {
  str(" "),
  str(":"),
  str("\r\n"),
  str("\r\n\r\n"),
  str("HTTP/1"),
  str("HTTP/1.1"),
  str("HTTP/2"),
  str("HTTP/3"),
};
