#pragma once

#include "networking/sockets/udp.hpp"
#include "common/pcinfo.hpp"

namespace Subservices::Discovery::Find
{
    bool find_manager(Networking::Sockets::UDP &conn, PC::new_pcs_queue &new_pcs);
}