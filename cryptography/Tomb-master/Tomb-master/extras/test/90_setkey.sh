#!/usr/bin/env zsh

export test_description="Testing set key"

source ./setup

test_export "test" # Using already generated tomb
test_expect_success 'Testing set key' '
    tt forge -k $tomb_key_new --tomb-pwd $DUMMYPASS \
        --ignore-swap --unsafe --use-urandom --force &&
    tt setkey -k $tomb_key_new $tomb_key $tomb \
        --unsafe --tomb-pwd $DUMMYPASS --tomb-old-pwd $DUMMYPASS &&
    tt open -k $tomb_key_new $tomb \
        --unsafe --tomb-pwd $DUMMYPASS &&
    print $DUMMYPASS \
        | gpg --batch --passphrase-fd 0 --no-tty --no-options -d $tomb_key_new \
        | hexdump -C &&
    tt_close
    '

test_export "recipient" # Using already generated tomb
test_expect_success 'Testing tomb with GnuPG keys: setkey' '
    tt forge $tomb_key_new -g -r $KEY2 --ignore-swap --unsafe --use-urandom &&
    tt setkey -k $tomb_key_new  $tomb_key $tomb -g -r $KEY2 &&
    tt open -k $tomb_key_new $tomb -g &&
    tt_close
    '

test_done
