# Probr TestDevice
This is a simple virtual machine, which is able to produce fake pcap files and can be used to test functionality of probr-core and probr-analysis.

It uses an Ubuntu 14.04 Vagrant image and a small program written in C++, which is able to simulate the functionality of tcpdump to some extend by creating periodic pcap files with probe requests.

# Prerequisites
1. Vagrant (Tested with 1.7.4 and Virtualbox 5)

# Usage
Please do not start the probr-core backend before having started the test device. The reason is, that in order to communicate from the guest machine to the host backend, a private network is created (192.168.50.0/24). The guest will take on 192.168.50.2 and the host 192.168.50.1. Therefore the host must bind to 192.168.50.1. If the necessairy network device has not yet been created by virtualbox, running the server on 0.0.0.0:PORT will not bind to 192.168.50.1.

```sh
git clone git@github.com:probr/probr-testdevice.git
cd probr-testdevice
vagrant up
```

This step might take a while. Vagrant provisions the machine and does the following steps automatically:
1. Install various packages including git, gcc-4.9, cmake and c++ libraries
2. Clone, compile and install [libtins](libtins), a C++ network packaet library
3. Compile and install the pcapgen.cpp program

Next make sure you have all the prerequisites from probr-core running. A mongo server, a redis server and a celery worker.

Next fire up probr-core and bind to the virtualbox network ip address.
```sh
python manage.py runserver 192.168.50.1:8000
```
or, if you want to use probr-core in other networks you can also bind to all interfaces with
```sh
python manage.py runserver 0.0.0.0:8000
```
Then add a new device to probr-core through your browser. Make sure you use the 192.168.50.1:8000 address, so that the generated snippet will have the correct ip address. Copy the snippet to the clipboard and SSH into the VM:
```sh
vagrant ssh
```
Paste the snippet
```sh
vagrant ssh
wget -qO- http://192.168.50.1:8000/static/bootstrap.sh | sh -s 06b78daa26f3a515c5542323051d3268dcab4f1ff1186a9b4fb867003bd0bcbf http://192.168.50.1:8000
```
The device should now post statuses to the backend on the host machine.

Now you can start a capture command and an upload command with the following command templates:

## Upload command
```sh
mkdir -p captures

# Upload Existing
for file in captures/*.pcap
do
    post_file '/api-device/device-captures/' "$file" && rm "$file" 
done

# Upload future pcaps (blocking)
inotifywait -m captures/ -e close_write -e move |
    while read path action file; do
        post_capture "$file" && rm "captures/$file" 
    done
```

## Pcap generation
```sh
mkdir - p captures
cd captures
pcapgen 0 5
```

Run pcapgen without arguments to learn how to use it:
```sh
pcapgen
pcapgen version 0.1
Usage: pcapgen count duration [meantimediff] [stdtimediff]
	count		Number of pcap files to be generated
			Use 0 for infinite pcap generation
	duration	How many seconds to record in one file
	meantimediff	Mean value for time in milliseconds between two probe requests
			Lower means more probe requests per second
			(Default 500)
	stdtimediff	Standard deviation for time in milliseconds between two probe requests
			(Default 1000)
```

# Halting, reloading and resetting
If you want to shut down the machine use
```sh
vagrant halt
```

If you want to restart the machine use
```sh
vagrant reload
```
If the device has been registered before, it will try to connect againt to the backend and post statuses again.

If you want to completely reset the device use:
```sh
vagrant destroy
```
