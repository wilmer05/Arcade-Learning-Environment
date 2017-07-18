# We do not automatically build the recording agent, which requires SDL. To build it, run
#
# > make recordingAgent
all: sharedLibraryAgent rlglueAgent fifoAgent iwAgent iwAgentRGB rolloutsAgent simulatorAgent

sharedLibraryAgent: 
	make -f Makefile.sharedlibrary

rlglueAgent:
	make -f Makefile.rlglue

fifoAgent: 
	make -f Makefile.fifo

recordingAgent: 
	make -f Makefile.recording

iwAgent:
	make -f Makefile.iwAgent

iwAgentRGB:
	make -f Makefile.iwAgentRGB

rolloutsAgent:
	make -f Makefile.rolloutsAgent

simulatorAgent:
	make -f Makefile.simulatorAgent

clean:
	make -f Makefile.rlglue clean
	make -f Makefile.sharedlibrary clean
	make -f Makefile.fifo clean
	make -f Makefile.recording clean
	make -f Makefile.iwAgent clean
	make -f Makefile.iwAgentRGB clean
	make -f Makefile.rolloutsAgent clean
	make -f Makefile.simulatorAgent clean

