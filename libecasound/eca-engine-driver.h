#ifndef INCLUDED_ECA_ENGINE_DRIVER_H
#define INCLUDED_ECA_ENGINE_DRIVER_H

class ECA_ENGINE;
class ECA_CHAINSETUP;

/**
 * Virtual base class for implementing ecasound
 * engine driver objects.
 *
 * Drivers are used to synchronize engine 
 * execution to external timing sources. 
 * For example soundcard's interrupt generation
 * can serve as a driver.
 *
 * @author Kai Vehmanen
 */
class ECA_ENGINE_DRIVER {

 public:

  /** @name Public API for driver execution */
  /*@{*/

  /**
   * Starts engine execution. 
   *
   * @pre engine != 0
   * @pre engine->is_valid() == true
   * @pre engine->connected_chainsetup() == csetup
   */
  virtual void exec(ECA_ENGINE* engine, ECA_CHAINSETUP* csetup) = 0;

  /*@}*/


  /** @name Public API for external requests */
  /*@{*/

  /**
   * Signals that driver should start operating 
   * the engine. Once started, driver is allowed
   * to call ECA_ENGINE functions.
   */
  virtual void start(void) = 0;

  /**
   * Signals that driver should stop operation.
   * Once stopped, driver must not call
   * any ECA_ENGINE functions.
   */
  virtual void stop(void) = 0;

  /**
   * Signals that driver should stop operation 
   * and return from its exec() method.
   * After exiting, driver must not call any 
   * ECA_ENGINE functions.
   */
  virtual void exit(void) = 0;

  /*@}*/

};

#endif /* INCLUDED_ECA_ENGINE_DRIVER_H */
