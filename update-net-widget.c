#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "part_net_if.h"

int nr_interfaces;
bool *ifs_link_up;
int *ifs_nr_addrs;

void init_if_structures()
{
    struct if_nameindex *if_ni, *i;

    if_ni = if_nameindex();
    if (if_ni == NULL) {
        perror("if_nameindex");
        exit(EXIT_FAILURE);
    }

    nr_interfaces = 0;
    for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++) {
        if (strcmp(i->if_name, "lo")) {
            nr_interfaces++;
        }
    }

    if_freenameindex(if_ni);

    ifs_link_up = calloc(nr_interfaces, sizeof(bool));
    ifs_nr_addrs = calloc(nr_interfaces, sizeof(int));
}

void send_netlink_request(int sock, int type)
{
    struct {
        struct nlmsghdr nlh;
        struct rtgenmsg msg;
    } req;

    req.nlh.nlmsg_len = sizeof(req);
    req.nlh.nlmsg_type = type;
    req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;

    send(sock, (void *) &req, sizeof(req), 0);
}

void update_widget()
{
    bool any_if_up = false;
    bool any_if_has_addr = false;
    int i;
    for (i = 0; i < nr_interfaces; i++) {
        if (ifs_link_up[i]) {
            any_if_up = true;
        }
        if (ifs_nr_addrs[i] > 0) {
            any_if_has_addr = true;
        }
    }

    char *status = "D";
    if (any_if_has_addr) {
        status = "U";
    } else if (any_if_up) {
        status = "C";
    }

    FILE *fp = popen("awesome-client", "w");
    fprintf(fp, "netwidget:set_text('%s ')", status);
    pclose(fp);
}

void process_netlink_msg(char *msg, int len)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *)msg;
    while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {

        if (nlh->nlmsg_type == RTM_NEWLINK
                || nlh->nlmsg_type == RTM_DELLINK) {
            struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(nlh);
            if (ifi->ifi_index != 1) {
                int if_index = ifi->ifi_index - 2;
                if (ifi->ifi_flags & IFF_UP
                        && ifi->ifi_flags & IFF_RUNNING
                        && ifi->ifi_flags & IFF_LOWER_UP) {
                    if (!ifs_link_up[if_index]) {
                        ifs_link_up[if_index] = true;
                        update_widget();
                    }
                } else {
                    if (ifs_link_up[if_index]) {
                        ifs_link_up[if_index] = false;
                        update_widget();
                    }
                }
            }
        }

        if (nlh->nlmsg_type == RTM_NEWADDR
                || nlh->nlmsg_type == RTM_DELADDR) {
            struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
            if (ifa->ifa_index != 1
                    && ifa->ifa_family == AF_INET
                    && ifa->ifa_scope == RT_SCOPE_UNIVERSE) {
                int if_index = ifa->ifa_index - 2;
                if (nlh->nlmsg_type == RTM_NEWADDR) {
                    ifs_nr_addrs[if_index]++;
                    if (ifs_nr_addrs[if_index] == 1) {
                        update_widget();
                    }
                } else {
                    ifs_nr_addrs[if_index]--;
                    if (ifs_nr_addrs[if_index] == 0) {
                        update_widget();
                    }
                }
            }
        }

        nlh = NLMSG_NEXT(nlh, len);
    }
}

void sig_handler(int signo)
{
    if (signo == SIGHUP) {
        update_widget();
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_nl addr;
    int sock, len;
    char buffer[4096];

    init_if_structures();

    if (signal(SIGHUP, sig_handler) == SIG_ERR) {
        perror("Can't catch SIGHUP");
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        perror("Couldn't open NETLINK_ROUTE socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Couldn't bind socket");
        exit(EXIT_FAILURE);
    }

    send_netlink_request(sock, RTM_GETLINK);
    len = recv(sock, buffer, 4096, 0);
    process_netlink_msg(buffer, len);
    send_netlink_request(sock, RTM_GETADDR);

    while ((len = recv(sock, buffer, 4096, 0)) > 0) {
        process_netlink_msg(buffer, len);
    }

    free(ifs_link_up);
    free(ifs_nr_addrs);
}
