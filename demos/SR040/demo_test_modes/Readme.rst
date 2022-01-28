..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _demo-sr040-test-modes:

=======================================================================
 demo_test_modes
=======================================================================

.. desc:start

This demo showcases on How to Start SR040 in Test Mode. In this demo,
you can select one of the following test modes.

- Continuous Wave Mode
- Loop Back Mode
- Loop Back And Save Mode
- Tx Only Mode
- Rx Only Mode

.. desc:end


Important Notes for RX/TX PER
=======================================================================

.. warning::

    When running RX/TX PER, the following things must be considerd.


1) For RX mode, at very high rate, it is advisable to define ``ENABLE_UCI_CMD_LOGGING``
   macro in :file:`phNxpLogApis_TmlUwb.h` as ``DISABLED`` so that incoming PHY LOG
   notifications are not missed/stalled while doing UCI logging.

.. literalinclude:: /libs/halimpl/inc/phNxpLogApis_TmlUwb.h
   :language: c
   :start-after: /* doc:start:uci-cmd-logging */
   :end-before: /* doc:end:uci-cmd-logging */

2) Optimazation should be enabled at host to highest level to speed up processing
   of notifications.

Configuration / selection
=======================================================================


The selection is done by, enabling one of these flags.

.. literalinclude:: demo_test_modes.c
    :language: c
    :start-after: /* doc-configure-demo-test-mode:start */
    :end-before: /* doc-configure-demo-test-mode:end */

For RX and TX Modes, additional settings are available.
They can be controlled via:

.. literalinclude:: demo_test_modes.c
    :language: c
    :start-after: /* Configuration for RX and TX Modes:start */
    :end-before: /* Configuration for RX and TX Modes:end */


Prerequisites
=======================================================================

Counterpart equipment like VSA


How to Run
=======================================================================

- Select the configuration
- Compile the project
- Setup counterpart equipment (litepoint/spectrum analyzer)
- Run this demo


PHY Log parsing
=======================================================================

This demo will generate a log file ``uwb_per_log_COMXX.log`` based on
log notifications received from SR040. This file can be parsed to see test
mode and PER statistics. See :numref:`phy-log-parsing` :ref:`phy-log-parsing`.


See Also
===============

Please see :numref:`sr040-test-modes` :ref:`sr040-test-modes`
