/*
 *  fagelmatare-dlogger
 *    Receives sensor data from master and logs it to MySQL database
 *  dlogger.c
 *    Initialize communication with master and listen for FG_SENSOR_DATA event 
 *****************************************************************************
 *  This file is part of Fågelmataren, an embedded project created to learn
 *  Linux and C. See <https://github.com/Linkaan/Fagelmatare>
 *  Copyright (C) 2015-2017 Linus Styrén
 *
 *  Fågelmataren is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the Licence, or
 *  (at your option) any later version.
 *
 *  Fågelmataren is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public Licence for more details.
 *
 *  You should have received a copy of the GNU General Public Licence
 *  along with Fågelmataren.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************
 */
#include <signal.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifdef FG_RELINC
#include "../serializer/events.h"
#include "../events/fgevents.h"
#else
#include <events.h>
#include <fgevents.h>
#endif

#include "common.h"
#include "log.h"

/* Forward declarations used in this file. */
static int fg_handle_event (void *, struct fgevent *, struct fgevent *);

/* Non-zero means we should exit the program as soon as possible */
static sem_t keep_going;

/* Signal handler for SIGINT, SIGHUP and SIGTERM */
static void
handle_sig (int signum)
{
    struct sigaction new_action;

    sem_post (&keep_going);

    new_action.sa_handler = handle_sig;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;

    sigaction (signum, &new_action, NULL);
}

/* Setup termination signals to exit gracefully */
static void
handle_signals ()
{
    struct sigaction new_action, old_action;

    /* Turn off buffering on stdout to directly write to log file */
    setvbuf (stdout, NULL, _IONBF, 0);

    /* Set up the structure to specify the new action. */
    new_action.sa_handler = handle_sig;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;
    
    /* Handle termination signals but avoid handling signals previously set
       to be ignored */
    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGINT, &new_action, NULL);
    sigaction (SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGHUP, &new_action, NULL);
    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGTERM, &new_action, NULL);
}

int
main (void)
{
    ssize_t s;    
    struct thread_data tdata;

    /* Initialize keep_going as binary semaphore initially 0 */
    sem_init (&keep_going, 0, 0);

    memset (&tdata, 0, sizeof (tdata));

    handle_signals ();

    s = fg_events_client_init_inet (&tdata.etdata, &fg_handle_event, NULL,
                                    &tdata, MASTER_IP, MASTER_PORT,
                                    FG_DATALOGGER);
    if (s != 0)
      {
        log_error_en (s, "error initializing fgevents");
        //return 1;
      }

    sem_wait (&keep_going);

    /* **************************************************************** */
    /*                                                                  */
    /*                      Begin shutdown sequence                     */
    /*                                                                  */
    /* **************************************************************** */

    fg_events_client_shutdown (&tdata.etdata);

    return 0;
}

static int
fg_handle_event (void *arg, struct fgevent *fgev,
                 struct fgevent * UNUSED(ansev))
{
    int i;
    struct thread_data *tdata = arg;

    /* Handle error in fgevent */
    if (fgev == NULL)
      {
        log_error_en (tdata->etdata.save_errno, tdata->etdata.error);
        return 0;
      }

    switch (fgev->id)
      {
        case FG_SENSOR_DATA:
            _log_debug ("eventid: %d\n", fgev->id);
            for (i = 0; i < fgev->length; i++)
              {
                _log_debug ("%d\n", fgev->payload[i]);
              }
            break;
        default:
            _log_debug ("eventid: %d\n", fgev->id);
            break;                                                          
      }
      
    return 0;
}
