#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
import demoscript.pexpect as pexpect

def run(client, server):
    print "testing client... ",
    sys.stdout.flush()
    client.sendline('i')
    server.expect('Hello World!')
    client.sendline('d')
    try:
        server.expect('Hello World!', timeout=1)
    except pexpect.TIMEOUT:
        pass
    client.sendline('i')
    server.expect('Hello World!')
    server.expect('Hello World!')
    print "ok"

    print "testing shutdown... ",
    sys.stdout.flush()
    client.sendline('d')
    client.sendline('s')
    server.waitTestSuccess()

    client.expect('RequestCanceledException')
    client.sendline('x')
    client.waitTestSuccess()
    print "ok"