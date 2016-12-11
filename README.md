USER INTERFACE FOR IRQBALANCE
=============================

This project provides an ncurses-based textual user interface to
[irqbalance](https://github.com/Irqbalance/irqbalance) (daemon responsible for
IRQ distribution on Linux systems).


Features
========

* Show how IRQs are distributed over CPUs at given moment
* Runtime setup of sleep interval, IRQ and CPU banning (this was only possible
to do during the start of irqbalance)


Notes
=====

* Shown data are fetched with screen changes (if the sleep interval hasn't
elapsed after change, only old tree can be shown -- it's not possible to predict
how irqbalance will distribute IRQs before the rebalancing actually happens)
