Vagrant.configure("2") do |config|
  config.vm.define "tcpserver" do |tcpserver|
    tcpserver.vm.box = "ubuntu/trusty64"
    tcpserver.vm.hostname = 'tcpserver'
    tcpserver.vm.box_url = "ubuntu/trusty64"

    tcpserver.vm.network :private_network, ip: "10.0.0.20"
    tcpserver.vm.network :forwarded_port, guest: 1050, host: 1200, protocol: "tcp"

    tcpserver.vm.provider :virtualbox do |v|
      v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--name", "tcpserver"]
    end
  end

  config.vm.define "tcpclient" do |tcpclient|
    tcpclient.vm.box = "ubuntu/trusty64"
    tcpclient.vm.hostname = 'tcpclient'
    tcpclient.vm.box_url = "ubuntu/trusty64"

    tcpclient.vm.network :private_network, ip: "10.0.0.21"
    tcpclient.vm.network :forwarded_port, guest: 1050, host: 1201, protocol: "tcp"

    tcpclient.vm.provider :virtualbox do |v|
      v.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--name", "tcpclient"]
    end
  end
  
  config.vm.provision "shell", inline: <<-SHELL
      apt-get update
      apt-get install -y g++
    SHELL
end