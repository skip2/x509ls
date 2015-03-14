A text-based SSL certificate viewer. Shows an SSL server's certificate chain, and the validation path formed by OpenSSL.

Similar to the certificate viewer found in web browsers, only more keyboard friendly.

![alt text](https://skip.org/x509ls/screenshot.png "x509ls screenshot")

The flags next to each certificate are:
 * <b><font color="red">s</font></b>: Self-signed certificate.
 * <b><font color="gold">t</font></b>: In the trust store. The trust store can be set with the <i>--capath</i> and <i>--cafile</i> options.
 * <b><font color="green">v</font></b>: In the validation path formed by OpenSSL.
 * <b><font color="purple">c</font></b>: In the server's certificate chain.

![alt text](https://skip.org/img/new.png "new!") Jan 2014 - Save full server chain / validation path to file function.

## Requirements

 * ncurses, glibc 2.9+, OpenSSL 1.0.0+.
 * Works with Ubuntu 12.04, RHEL 6 okay.

For Ubuntu run:
```
 sudo apt-get install cmake make g++ libncurses5-dev libssl-dev
```

For RHEL/CentOS run:
```
 sudo yum install cmake gcc-c++ ncurses-devel openssl-devel
```

### Build & test run
```
 cmake .
 make

 x509ls/x509ls
```

### Install
```
 sudo make install
```

### Trivia

 * The longest server chain I've found contains some 108 certificates(!). 107 of these certificates are not even required, since the end-entity certificate is self-signed anyway.

![alt text](https://skip.org/x509ls/long_server_chain.png "long server chain screenshot")

