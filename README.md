# What is this repository for? #

Ns-3 module for mesh grid dense sensor networks and network-on-chip like network architectures.

It integrates with computational fluid dynamics to provide spatial and temporal data to the sensors in the network. We implement post-processing tools for packet and flow tracing, statistical survey and qualitative analysis of the sensed and processed data.

Following link provides videos with demos of the visualizing tools developed:
https://www.youtube.com/playlist?list=PLbRUoHMnHw7R9h8Hv8kuqBF7qa3Vf9whM

https://youtu.be/26GKSbZVWsU

### More details in the [wiki](https://bitbucket.org/joaofl/noc/wiki/Home). ###

**Scentific papers presented:**

J. Loureiro, V. Gupta, N. Pereira, E. Tovar, and R. Rangarajan, “XDense: A sensor network for extreme dense sensing,” Proceedings of the Work-In-Progress Session at the 2013 IEEE Real-Time Systems Symposium - RTSS, pp. 19–20, 2013.

J. Loureiro, R. Rangarajan, and E. Tovar, “Demo abstract: Towards the development of XDense, a sensor network for dense sensing,” 12th European Conference on Wireless Sensor Networks (EWSN), p. 23, 2015.

J. Loureiro, R. Rangarajan, and E. Tovar, “XDense: A dense grid sensor network for distributed feature extraction,” 33o Simpósio Brasileiro de Redes de Computadores e Sistemas Distribuídos (SBRC 2015) - Workshop de Comunicação em Sistemas Embarcados Críticos (WoCCES), 2015.

J. Loureiro, M. Albano, T. Cerqueira, R.R. Rangarajan, E. Tovar, “A module for the XDense architecture in ns-3”, Workshop on ns-3 (WNS3 ‘15), May 13th, 2015, pp. xxxxx, Castelldefels, Spain

J. Loureiro, R. Rangarajan, and E. Tovar, “Distributed sensing of fluid dynamic phenomena with the XDense sensor grid network,” 3rd IEEE International Conference on Cyber-Physical Systems, Networks and Applications (CPSNA 2015), Hong Kong.


# How do I get set up? #

To set it up, place the module inside the **src** dir of ns-3, configure ns-3 and build it. Check the input parameters to run the example.

Tested in version 3.23 of ns-3. Testers not implemented yet.

Dependencies of the GUI (you need to install them before executing the user interface):

- python-numpy

- python-mathplotlib

- python-joblib

-libarmadillo-dev


# Who do I talk to? #

Authors:
João Loureiro

Team members:
Pedro Santos
