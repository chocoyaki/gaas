/****************************************************************************/
/* The classes representing the nodes of a functional workflow              */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */

#ifndef _FNODE_HH_
#define _FNODE_HH_

#include "Node.hh"
#include "FNodePort.hh"
#include "WfUtils.hh"
class FWorkflow;

using namespace std;

/*****************************************************************************/
/*                                 FNode                                     */
/*****************************************************************************/

class FNode : public Node {

public:
  FNode(FWorkflow* wf, const string& id, short maxInstances);
  virtual ~FNode();

  virtual WfPort *
  newPort(string portId,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth);
  /**
   * instanciation
   */

  virtual void
  instanciate(Dag* dag) = 0;

  bool
  isOnHold();

  void
  resumeInstanciation();

  bool
  isFullyInstantiated();

protected:
  FWorkflow * wf;

  /**
   * Status 'on hold' is used when instanciation is stopped
   */
  bool isOnHoldFlag;

  /**
   * Status 'fully instantiated' used when instanciation is completed
   */
  bool isFullInst;

  /**
   * Max number of instances per dag
   */
  short maxInstNb;

}; // end class FNode

/*****************************************************************************/
/*                           FConstantNode                                   */
/*****************************************************************************/

class FConstantNode : public FNode {

  public:
    FConstantNode(FWorkflow* wf,
                  const string& id,
                  WfCst::WfDataType type);
    virtual ~FConstantNode();
    void setValue(const string& strVal);

    virtual void instanciate(Dag* dag); // parameter is not used

  private:
    string myValue;
    static string outPortName;
}; // end class FConstantNode

/*****************************************************************************/
/*                            FSourceNode                                    */
/*****************************************************************************/

class FSourceNode : public FNode {

  public:
    FSourceNode(FWorkflow* wf,
                const string& id,
                WfCst::WfDataType type,
                short maxInstances);
    virtual ~FSourceNode();

    virtual void instanciate(Dag* dag); // parameter is not used

  private:

    static string outPortName;
}; // end class FSourceNode

/*****************************************************************************/
/*                             FProcNode                                     */
/*****************************************************************************/

class FProcNode : public FNode {

  public:
    FProcNode(FWorkflow* wf,
              const string& id,
              short maxInstances);
    virtual ~FProcNode();

    virtual void instanciate(Dag* dag);

    void
    setDIETServicePath(const string& path);

  protected:

    string myPath;

}; // end class FProcNode

/*****************************************************************************/
/*                           FConditionNode                                  */
/*****************************************************************************/

class FConditionNode : public FNode {

  public:
    FConditionNode(FWorkflow* wf, const string& id);
    virtual ~FConditionNode();

    virtual void instanciate(Dag* dag);
}; // end class FConditionNode

#endif // _FNODE_HH_
