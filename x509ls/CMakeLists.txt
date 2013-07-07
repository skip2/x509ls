CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_CXX_FLAGS "-g -O2 -Wall -Werror -pedantic-errors")

SET(CURSES_NEED_NCURSES TRUE)
FIND_PACKAGE(Curses REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)

INCLUDE_DIRECTORIES(${CURSES_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(../)

SET(LIBS -lncurses -lpanel -lanl ${OPENSSL_LIBRARIES})

SET(SOURCES
  # Main top level application.
  main.cc                        # *main(), command line options parsing*.
  x509ls.cc                    # *Main application*.

  # Certificate handing.
  certificate/certificate.cc     # A single X509 certificate.
  certificate/certificate_list.cc # A list of X509 certificates.
  certificate/trust_store.cc     # Trust store (trusted certificates) wrapper.

  # Lowest level objects.
  base/base_object.cc            # Base class, can send/emit events, watch FDs.
  base/event_manager.cc          # Event publish/subscribe mechanism.
  base/openssl/openssl_environment.cc # OpenSSL setup/teardown.
  base/openssl/bio_translator.cc  # OpenSSL memory BIO w/ std::string accessor.

  # CLI base classes.
  cli/base/cli_application.cc    # Base application class, *main event loop*.
  cli/base/cli_control.cc        # Base class for CLI controls.

  # CLI controls & styles.
  cli/base/colours.cc            # Terminal colour definitions.
  cli/base/command_line.cc       # CLI control: single line text entry field.
  cli/base/info_bar.cc           # CLI control: single line status/info bar.
  cli/base/list_control.cc       # CLI control: scrollable list of items.
  cli/base/text_control.cc       # CLI control: scrollable text viewer.

  # X509LS specific CLI controls.
  cli/menu_bar.cc                # CLI control: menu bar w/x509ls "logo".
  cli/certificate_list_layout.cc # CLI layout: list&preview of certificates.
  cli/certificate_list_control.cc # CLI control: list of certificates.
  cli/certificate_view_layout.cc # CLI control: single fullscreen certificate.
  cli/status_bar.cc              # CLI control: simple status bar.

  # Networking.
  net/dns_lookup.cc              # A single asynchronous DNS lookup.
  net/chain_fetcher.cc           # Coordinates TLS, uses DNSLookup & TLSClient.
  net/ssl_client.cc              # Fetches certificates.
)

ADD_EXECUTABLE(x509ls ${SOURCES})
TARGET_LINK_LIBRARIES(x509ls ${LIBS})

FILE(DOWNLOAD
  http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
  ${CMAKE_BINARY_DIR}/cpplint.py
)

ADD_CUSTOM_TARGET(Lint ALL ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/cpplint.py
  --root=${CMAKE_SOURCE_DIR}
  x509ls/*.h x509ls/*.cc
  x509ls/*/*.h x509ls/*/*.cc
  x509ls/*/*/*.h x509ls/*/*/*.cc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

INSTALL(TARGETS x509ls DESTINATION bin)

