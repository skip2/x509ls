A text-based SSL certificate viewer. Shows an SSL server's certificate chain, and the validation path formed by OpenSSL.

Similar to the certificate viewer found in web browsers, only more keyboard friendly.

![https://skip.org/x509ls/screenshot.png](https://skip.org/x509ls/screenshot.png)

The flags next to each certificate are:
  * <b><font color='red'>s</font></b>: Self-signed certificate.
  * <b><font color='gold'>t</font></b>: In the trust store. The trust store can be set with the <i>--capath</i> and <i>--cafile</i> options.
  * <b><font color='green'>v</font></b>: In the validation path formed by OpenSSL.
  * <b><font color='purple'>c</font></b>: In the server's certificate chain.

![https://skip.org/img/new.png](https://skip.org/img/new.png) - Jan 2014 - Save full server chain / validation path to file function.

## Requirements ##

  * ncurses, glibc 2.9+, OpenSSL 1.0.0+.
  * Works with Ubuntu 11.04, CentOS 6 okay.

## Trivia ##

  * The longest server chain I've found contains some 108 certificates(!). 107 of these certificates are not even required, since the end-entity certificate is self-signed anyway.

![https://skip.org/x509ls/long_server_chain.png](https://skip.org/x509ls/long_server_chain.png)