/*!

        \file                                    Visitor.h
        \brief                                  Generic Visitor Class
        \author                                Georg AUZINGER
        \version                               1.0
        \date                                   07/10/14
        Support :                              mail to : georg.auzinger@cern.ch
 */


#ifndef Visitor_h__
#define Visitor_h__

#include <stdint.h>

namespace Ph2_System
{
	class SystemController;
}

namespace GUI
{
	class SystemControllerWorker;
}

namespace Ph2_HwDescription
{
	class BeBoard;
	class Module;
	class Cbc;
}

namespace Ph2_HwInterface
{
	class Event;
}

class HwDescriptionVisitor
{
  public:
	virtual ~HwDescriptionVisitor(){}
	/*!
	 * \brief Visitor for top level System Controller
	 * \param pSystemController
	 */
	virtual void visit( Ph2_System::SystemController& pSystemController ) {}
	// virtual void visit() = 0;

	/*!
		 * \brief Visitor for top level System Controller in the GUI
		 * \param pSystemController
		 */
	virtual void visit( const GUI::SystemControllerWorker& pSystemControllerWorker ) {}

	/*!
	 * \brief Visitor for BeBoard Class
	 * \param pBeBoard
	 */
	virtual void visit( Ph2_HwDescription::BeBoard& pBeBoard ) {}
	/*!
	 * \brief Visitor for Module Class
	 * \param pModule
	 */
	virtual void visit( Ph2_HwDescription::Module& pModule ) {}
	/*!
	 * \brief Visitor for Cbc Class
	 * \param pCbc
	 */
	virtual void visit( Ph2_HwDescription::Cbc& pCbc ) {}
};

class HwInterfaceVisitor
{
    public:
	virtual ~HwInterfaceVisitor(){}
	virtual void visit ( const Ph2_HwInterface::Event& pEvent ) = 0;
};

#endif
