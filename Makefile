LD = -L/usr/local/lib
LIBS = -lboost_serialization -lboost_system -lboost_program_options
SOURCEDIR = ./SOURCE
OBJ = KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o
OBJS = $(addprefix ${SOURCEDIR}/, $(OBJ))
HD = Distance.hpp
HDS = $(addprefix ${SOURCEDIR}/, $(HD))
OPT = --std=c++14 -O2

ki: ${OBJS}
	${CXX} ${OPT} ${SOURCEDIR}/KILM_main.cpp ${OBJS} ${LD} ${LIBS} -o ${SOURCEDIR}/kilm.exe

$(SOURCEDIR)/%.o: %.cpp
	@[ -d $(SOURCEDIR/) ]
	${CXX} ${OPT} ${LD} ${LIBS} -o $@ -c $<

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
	rm -f ${SOURCEDIR}/*.o ${SOURCEDIR}/*.dump ${SOURCEDIR}/*.exe ${SOURCEDIR}/*.log
