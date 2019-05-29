# taken (slightly modified) from https://stackoverflow.com/a/20828366
# All credit goes to Tanner Sansbury


from time import sleep
import signal_example

def spam1(x):
    print "spam1: ", x

def spam2(x):
    print "spam2: ", x

c = signal_example.MyClass()
c.connect_slot(spam1)
c.connect_slot(spam2)
c.event(123)
print "Sleeping"
c.event_in(3, 321)
sleep(5)
print "Done sleeping"