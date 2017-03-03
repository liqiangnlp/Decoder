#!/bin/bash

export LD_LIBRARY_PATH=`pwd`/../lib
./job_module_scheduler_server ../config/job.server/job.tcp.server.config

