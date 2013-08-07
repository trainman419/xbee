# XBee

Tools for interacting with Digi Xbee modules in API mode.

## xbsh2

The updated xbee shell, for interacting with local and remote Xbee modules in API mode. Includes history support and custom tab-completion.

Usage:

    $ xbsh2 <port> [baud]
    xbsh>

Commands mostly mirror the xbee AT command set, but are wrapped in more user-friendly command names and enumerations.

Complete command documentation is not available yet, but the command listing is availble through tab-completion.

There is a framework for an online help system, but the help data has not been filled out yet.


## xbsh

The original xbee shell, a very simple interface for sending AT commands to local and remote xbee modules in API mode.

Usage:

    $ xbsh <port>
    xbsh> 

Send raw AT commands with ATXX:

    xbsh> ATD0 

Set remote address with address:

    xbsh> address 0012:3456:7893:3452

Send remote AT commands with r

    xbsh> rD0


# Tutorials

## Building

Install the serial library from: https://github.com/wjwwood/serial

Build:

    make


Installing: there is no install target yet; simply copy the binary into your system bin directory:

    sudo cp xbsh2 /usr/bin


## Local Usage

Assuming your module is running the default configuration with API firmware at 9600 baud, on /dev/ttyUSB0

    $ xbsh2 /dev/ttyUSB0

To get the configuration of digital/analog IO pin 0:

    xbsh> io D0
    comissioning

To set the configuration of digital/analog IO pin 0:

    xbsh> io D0 analog-in
    Success

To get the readings for all of the sample pins:

    xbsh> io force-sample
    DIO1: 1
    AD0: 527

## Remote Usage

xbsh2 can be used to communicate with and configure remote xbees, using the same commands that are used for a local xbee. You must have an xbee connected to your computer that is part of the network.

To discover other nodes, use discover-nodes:

    xbsh> discover-nodes
    ( example coming soon )

To connect to a remote node, use the remote command:

    xbsh> remote 0023:0345:4594:1923

Now, all commands that configure xbee will communicate with the remote xbee instead of your local xbee:

    xbsh> io D0
    comissioning

To resume communicating with your local radio, use remote pop:

    xbsh> remote pop

The list of remote nodes is a stack, and remote commands go to the address at the top of the stack. To view the stack, use remote:

    xbsh> remote
    ( example coming soon )

## Debug Mode

Debug mode prints an enormous amount of debug information about trasmitted bytes, received bytes, packetization and parsing.

    xbsh> debug on
    debugging on
    xbsh> debug off
    debugging off
