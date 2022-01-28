..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr040-demo-tracker-ble:

=======================================================================
 demo_tracker_ble
=======================================================================

.. brief:start

This demo showcases App ranging via BLE with SR040 configured as a Controller - Initiator
and Helios configured as a Controllee - Responder.

For details on Peer-to-Peer ranging sequence and configuration details
for Helios and SR040, refer to :ref:`p2p-ranging`.

.. brief:end

App Ranging with BLE on QN9090-SR040
=======================================================================

In this demo, an Android smartphone sends commands over BLE to QN9090
to initialize and configure session and start ranging. Helios is connected
to the Android smartphone via a type-C USB cable. The APK would configure
both devices and start ranging.

.. note:: When we close the session from APK, the application will force
    the device in to HPD to reduce power consumption.

Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Helios programmed with CDC app configured as a Controllee - Responder
- Helios should be connected to Android Smartphone using Type-C cable.


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the BLE based Ranging project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_tracker_ble``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_tracker_ble`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040 natively on QN9090.

1. Flash ``demo_tracker_ble-SR040.bin`` on QN9090.
#. Flash ``cdc_app.bin`` on Helios.
#. Connect Helios board to Android Smartphone using Type-C cable.
#. Install ``Find it!`` located at :file:`binaries/APK/` on Android Smartphone.
#. Enable Bluetooth & location.
#. Open ``Find it!`` App and add Tag.
#. Once tag is connected over Bluetooth, ranging will start automatically.


.. Refer below images for successfull Ranging!!!

.. image
