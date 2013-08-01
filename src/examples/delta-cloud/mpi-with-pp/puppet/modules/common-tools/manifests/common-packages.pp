
class common-tools::common-packages {

	package{"openmpi-bin": ensure => present}
	package{"libopenmpi-dev": ensure => present}
	package{"gcc": ensure => present}
}


