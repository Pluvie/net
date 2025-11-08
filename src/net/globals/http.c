static struct {
  str Space;
  str Colon;
  str Crlf;
  str Terminator;
  str Version_1;
  str Version_1_1;
  str Version_2;
  str Version_3;
  str Host;
  str Content_Length;
  str Transfer_Encoding;
  str Chunked_Transfer;
} Http = {
  str(" "),
  str(":"),
  str("\r\n"),
  str("\r\n\r\n"),
  str("HTTP/1"),
  str("HTTP/1.1"),
  str("HTTP/2"),
  str("HTTP/3"),
  str("Host"),
  str("Content-Length"),
  str("Transfer-Encoding"),
  str("chunked"),
};
