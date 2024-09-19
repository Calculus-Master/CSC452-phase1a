Vagrant.configure("2") do |config|

  config.vm.box = "bento/ubuntu-20.04-arm64"
  config.vm.provision :shell, path: "provision.sh"

  # CLion relies on a fixed SSH address, so assign static SSH port here. This
  # must be unique on the host machine.
  port = 12345
  config.vm.post_up_message = "\nUse port #{port} for CLion remote host connection"
  config.vm.network "forwarded_port", id: "ssh", host: port, guest: 22, auto_correct: false

end
