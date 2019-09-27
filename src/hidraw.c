/*
 *  Copyright 2016 Justin Schneck
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <poll.h>
#include <err.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <linux/types.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "erlcmd.h"
#include "hidraw_enum.h"

static const char notification_id = 'n';
static const char descriptor_id = 'd';
static const char error_id = 'e';

void device_handle_request(const char *req, void *cookie) {
  debug("Erl sent data");
}

void device_process(int fd) {
  debug("Read");
  int i, res = 0;
  char buf[1024];

  /* Get a report from the device */
  res = read(fd, buf, 1024);
  if (res < 0) {
    debug("read");
  } else {
    debug("read() read %d bytes:\n\t", res);
    char resp[res * 64];
    int resp_index = sizeof(uint16_t); // Space for payload size

    debug("%s", buf);

    resp[resp_index++] = notification_id;
    ei_encode_version(resp, &resp_index);
    ei_encode_tuple_header(resp, &resp_index, 2);
    ei_encode_atom(resp, &resp_index, "data");

    ei_encode_binary(resp, &resp_index, buf, res);
    erlcmd_send(resp, resp_index);
  }
}

void device_closed(int fd) {
  debug("device closed");
  char resp[16];
  int resp_index = sizeof(uint16_t); // Space for payload size

  resp[resp_index++] = error_id;

  ei_encode_version(resp, &resp_index);
  ei_encode_tuple_header(resp, &resp_index, 2);
  ei_encode_atom(resp, &resp_index, "error");
  ei_encode_atom(resp, &resp_index, "closed");
  erlcmd_send(resp, resp_index);
}

static int open_device(char *dev) {
  int fd;
  int i, res, desc_size = 0;
  char buf[256];

  struct hidraw_report_descriptor rpt_desc;
  struct hidraw_devinfo info;

  memset(&rpt_desc, 0x0, sizeof(rpt_desc));
  memset(&info, 0x0, sizeof(info));
  memset(buf, 0x0, sizeof(buf));

  debug("Open device %s", dev);
  fd = open(dev, O_RDWR|O_NONBLOCK);
  if (errno == EACCES && getuid() != 0)
    err(EXIT_FAILURE, "You do not have access to %s.", dev);

    /* Get Report Descriptor Size */
    res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
    if (res < 0)
      perror("HIDIOCGRDESCSIZE");
    else
      debug("Report Descriptor Size: %d\n", desc_size);
    /* Get Report Descriptor */
    rpt_desc.size = desc_size;
    res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
    if (res < 0) {
      perror("HIDIOCGRDESC");
    }

    debug("%s ", rpt_desc.value);

  struct erlcmd handler;
  erlcmd_init(&handler, device_handle_request, &fd);

  char resp[desc_size * 64];
  int resp_index = sizeof(uint16_t); // Space for payload size
  resp[resp_index++] = descriptor_id;
  ei_encode_version(resp, &resp_index);
  ei_encode_tuple_header(resp, &resp_index, 2);
  ei_encode_atom(resp, &resp_index, "descriptor");

  ei_encode_binary(resp, &resp_index, rpt_desc.value, desc_size);
  erlcmd_send(resp, resp_index);

  for (;;) {
    struct pollfd fdset[2];

    fdset[0].fd = STDIN_FILENO;
    fdset[0].events = POLLIN;
    fdset[0].revents = 0;

    fdset[1].fd = fd;
    fdset[1].events = (POLLIN | POLLPRI | POLLHUP);
    fdset[1].revents = 0;

    int timeout = -1; // Wait forever unless told by otherwise
    int rc = poll(fdset, 2, timeout);

    if (fdset[0].revents & (POLLIN | POLLHUP))
      erlcmd_process(&handler);

    if (fdset[1].revents & POLLIN)
      device_process(fd);

    if (fdset[1].revents & POLLHUP) {
      device_closed(fd);
      break;
    }
  }
  debug("Exit");
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp(argv[1], "enumerate") == 0)
    return enum_devices();
  else
    return open_device(strdup(argv[argc - 1]));
}
