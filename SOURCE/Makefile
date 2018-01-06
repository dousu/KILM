LD = -L/usr/local/lib
LIBS = -lboost_serialization -lboost_program_options -lboost_system
OBJ = KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o
HD = Distance.hpp
OPT = --std=c++14 -g -O2
CXX = g++

ki: ${OBJ}
	${CXX} ${OPT} KILM_main.cpp ${OBJ} ${LD} ${LIBS} -o kilm.exe

.cpp.o:
	${CXX} ${OPT} -c $<

KILM_main.cpp: KirbyAgent.o Rule.o Element.o LogBox.o Parameters.o MT19937.o ${HDS} KILM_main.h
KirbyAgent.o: KnowledgeBase.o LogBox.o KirbyAgent.h
KnowledgeBase.o: ${HDS} Rule.o IndexFactory.o Prefices.o LogBox.o KnowledgeBase.h
Rule.o: Element.o Dictionary.o IndexFactory.o Prefices.o Rule.h
Element.o:Dictionary.o IndexFactory.o Prefices.o Element.h
Dictionary.o:Dictionary.h
IndexFactory.o:IndexFactory.h
Prefices.o:Prefices.h
LogBox.o:LogBox.h
MT19937.o:MT19937.h
Parameters.o:KnowledgeBase.o Parameters.h

clean:
	rm -f *.o *.dump *.exe *.log
