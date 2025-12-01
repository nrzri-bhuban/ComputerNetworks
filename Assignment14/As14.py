from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSKernelSwitch, RemoteController
from mininet.cli import CLI
from mininet.log import setLogLevel

class LeafSpineTopo(Topo):
    """
    A Leaf-Spine topology scalable by switch radix (k).
    Reference: Similar to a 2-tier Fat-Tree.
    """

    def build(self, k=4):
        """
        k: Switch Radix (number of ports per switch). 
           Default is 4. k should ideally be an even number.
        """
        
        # Validate k (Leaf switches need to split ports evenly up/down)
        if k % 2 != 0:
            print(f"Warning: Radix k={k} is odd. Integer division will be used.")

        # --- Calculate Topology Dimensions based on Radix ---
        # 1. Each Spine connects to ALL Leaves. Since Spine has k ports:
        num_leaves = k
        
        # 2. Each Leaf connects to ALL Spines using half its ports (k/2):
        num_spines = k // 2
        
        # 3. Remaining Leaf ports (k/2) connect to Hosts:
        hosts_per_leaf = k // 2

        print(f"Building Topology with Radix k={k}")
        print(f"- Spines: {num_spines}")
        print(f"- Leaves: {num_leaves}")
        print(f"- Hosts per Leaf: {hosts_per_leaf}")
        print(f"- Total Hosts: {num_leaves * hosts_per_leaf}")

        spines = []
        leaves = []

        # --- Create Spine Switches ---
        # Naming convention: s1, s2...
        for i in range(num_spines):
            spine_sw = self.addSwitch(f'spine{i+1}')
            spines.append(spine_sw)

        # --- Create Leaf Switches and Connect to Spines ---
        # Naming convention: l1, l2...
        for i in range(num_leaves):
            leaf_sw = self.addSwitch(f'leaf{i+1}')
            leaves.append(leaf_sw)

            # Connect this Leaf to EVERY Spine (Mesh connection)
            for spine_sw in spines:
                self.addLink(leaf_sw, spine_sw)

            # --- Create Hosts and Connect to this Leaf ---
            # Naming convention: h_l1_1 (Host 1 on Leaf 1)
            for h in range(hosts_per_leaf):
                host = self.addHost(f'h_l{i+1}_{h+1}')
                self.addLink(leaf_sw, host)

# This dictionary enables Mininet to read the custom class
topos = { 'leafspine': ( lambda k=4: LeafSpineTopo(k=int(k)) ) }
