"""
This settings defines the maximum number
of headers can be sent in a single http
request to the target server.

Most of http servers allow up to 100 headers
per HTTP request, therefore, if the server
administrator have configured it to a lower
limit, execution of requests can fail and
lead to an http error 500 or something else.
"""
import objects


type = objects.buffers.RandLineBuffer


def setter(value):
    if 10 <= int(value) <= 680:
        return int(value)
    raise ValueError("must be an integer from 10 to 680")


def default_value():
    return 100
