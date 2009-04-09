/****************************************************************************/
/* The class representing the nodes of a functional workflow that will      */
/* correspond to tasks in the DAG
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/04/09 09:55:27  bisnard
 * new class
 *
 */

#ifndef _FACTIVITYNODE_HH_
#define _FACTIVITYNODE_HH_

#include "FNode.hh"

class FActivityNode : public FProcNode {

  public:

    FActivityNode(FWorkflow* wf, const string& id);
    virtual ~FActivityNode();

    /**
     * Set the maximum nb of instances of this node inside one dag
     * @param maxInst the max nb of instances
     */
    void
        setMaxInstancePerDag(short maxInst);

    /**
     * Set DIET service name
     * @param path the DIET service name
     */
    void
        setDIETServicePath(const string& path);

    /**
     * Set DIET estimation option
     * If estimOption == "constant" the instanciator will use the
     * 'est-class' attribute for all generated dag nodes
     * @param estimOption = ('constant'|...)
     */
    void
        setDIETEstimationOption(const string& estimOption);

    /**
     * Initialization
     *  - connection to other nodes
     *  - input iterators setup
     */
    virtual void
        initialize();

    /**
     * Instanciation of the activity as a DagNode
     * @param dag ref to the dag that will contain the DagNode
     */

    virtual void
        initInstanciation();

    virtual bool
        instLimitReached();

    virtual void
        createInstance(Dag* dag,
                       const FDataTag& currTag,
                       const vector<FDataHandle*>& currDataLine);

  protected:

    /**
     * The service path
     */
    string myPath;

    /**
     * The estimation option
     */
    string myEstimOption;

    /**
     * Max number of instances per dag
     */
    short maxInstNb;

  private:

    /**
     * Nb of instances created in current dag
     */
    int nbInstances;

};

#endif // _FACTIVITYNODE_HH_






