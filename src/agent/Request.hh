/**
 * @file  Request.hh
 *
 * @brief  Request class header
 *
 * @author   Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _REQUEST_HH_
#define _REQUEST_HH_

#include "DIET_data_internal.hh"
#include "GlobalSchedulers.hh"
#include "ms_function.hh"




class Request {
public:
  /**
   * Creates a new Request. Uses GlobalScheduler::chooseGlobalScheduler to set
   * GS member.
   *
   * @param request is the request to be presseced. There is no copy
   * of it and the variable used to initiate the Request must not be
   * modified or release before the Request is destroyed.
   */
  explicit
  Request(const corba_request_t *request);

  /**
   * Creates a new Request.
   *
   * @param request the request to be presseced. There is no copy of it and the
   *                variable used to initiate the Request must not be modified
   *                or release before the Request is destroyed.
   * @param GS the GlobalScheduler used to schedule servers for this request.
   */
  Request(const corba_request_t * request, GlobalScheduler * GS);


  /**
   * Destroys the Request. The corba_request_t used for creates the
   * Request can now be modified or released.
   */
  ~Request();

  /**
   * locks the Request mutex.
   */
  void
  lock();

  /**
   * unlock the Request mutex.
   */
  void
  unlock();

  /**
   * Waits that \c numberOfReponses sons send a reponses to it. The
   * reponseq are given by the \c addReponses() method. The \c lock()
   * method must be call before the call of \c waitReponses() and the
   * \c unlock() method after.
   *
   * @param numberOfReponses is the number of reponses waited. It must
   * be positive and not null.
   */
  void
  waitResponses(int numberOfReponses);

  /**
   * Add the \c response to the responses list. If this is the last
   * waited response, the \c waitResponses() methods stop to
   * wait. This method must not be called if the number of responses
   * is already get. The \c lock() and \c unlock() methods must be
   * call before and after the call to \c addResponse().
   *
   * @param response the response added to the responses list of the
   * \c Request.
   */
  void
  addResponse(const corba_response_t *response);


  /**
   * returns the list of responses send to the \c Request. The data is
   * own by the \c Request and free at the same time as the \c
   * Request.
   */
  corba_response_t *
  getResponses() const;


  /**
   * returns the number of reponses.
   */
  size_t
  getResponsesSize() const;

  /**
   * Returns the number of parameters of the request.
   */
  CORBA::Long
  getNumberOfParameters() const;

  /**
   * returns the request used to creates the Request.
   */
  const corba_request_t *
  getRequest() const;

  /**
   * Returns the scheduler associated to this request.
   */
  GlobalScheduler *
  getScheduler();

private:
  /**
   * frees the memory of responses and does responses = NULL;
   */
  void
  freeResponses();

  /**
   * This is the request which are processed. This is not a clone of
   * the \c corba_request_t used to creats the Request but the
   * \c corba_request_t itself.
   */
  const corba_request_t *request;

  /**
   * a mutex use by the gatheringEnded omni_condition.
   */
  omni_mutex respMutex;

  /**
   * An omni_condition used to wait all the responses of the request.
   */
  omni_condition *gatheringEnded;

  /**
   * number of waited responses.
   */
  int nbWaitedSonsResponse;

  /**
   * the responses given by the sons.
   */
  corba_response_t *responses;

  /**
   * size of responses
   */
  size_t responsesSize;

  /**
   * the scheduler to use for this request
   */
  GlobalScheduler *GS;
};

#endif  // _REQUEST_HH_
