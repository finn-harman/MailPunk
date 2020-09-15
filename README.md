# MailPunk

In this project, I model a realistic software development project which is often characterized by the use of libraries that are not under your control. The high-level objective of the project is to develop an email client. To keep things simple, the client shall have only limited functionality: logging in to a mail server (using the IMAP protocol), viewing emails and allowing their deletion. The application has three distinct components: the User Interface, the Control Logic and the Mailserver Interface.

My project is restricted to the middle part of the application: the control logic that manages the state of the application while it is running. This includes concrete entities such as mails, their body or attributes as well as logical entities such as connection handles. While embedding the control logic like this this limits my coding effort, it forces me to adhere to a strictly defined programming interface (i.e., functions I need to provide and functions I can use): interfacing to the UI component, I need to implement a given set of member functions. Interfacing with the mailserver, I have to work with the library I decided to use (libetpan). Specifically, the user interface is implemented as a terminal/console based application using the FinalCut library. The connection to the IMAP server is implemented using the libetpan library.

### Example use

Example use:
```
$ git clone https://github.com/finn-harman/MailPunk.git
$ cd MailPunk
$ mkdir build
$ cd build/
$ cmake ..
$ cmake --build .
$ cd ..
$ ./mailpunk
```

## Authors

* **Finn Harman** - [GitHub](https://github.com/finn-harman)
