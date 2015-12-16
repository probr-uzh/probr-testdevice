Vagrant.configure(2) do |config|
  config.vm.provider "virtualbox"
  config.vm.box = "ubuntu/trusty32"

  config.vm.network :private_network, ip: "192.168.100.31"

  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get install -y git make cmake libpcap-dev libssl-dev software-properties-common python-software-properties
    add-apt-repository -y ppa:ubuntu-toolchain-r/test
    apt-get update
    apt-get install -y g++-4.9
    git clone https://github.com/mfontanini/libtins.git
    cd libtins
    mkdir build
    cd build
    cmake ../ -DLIBTINS_ENABLE_CXX11=1 -DCMAKE_CXX_COMPILER=/usr/bin/g++-4.9
    make
    make install
    ldconfig
    cd /vagrant
    g++-4.9 pcapgen.cpp -ltins -std=c++11 -o pcapgen
    mv pcapgen /usr/bin
    sudo apt-get install -y inotify-tools
    touch /var/lib/cloud/instance/locale-check.skip
   SHELL
end
