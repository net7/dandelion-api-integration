<?php

/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 23/03/15
 * Time: 11:25
 */
class DataTxtApiService {

  private $api_endpoint;
  private $app_id;
  private $app_key;
  protected static $instance = NULL;
  private static $api_endpoints = array('iptc'=>'/datatxt/cl/beta','classification_api' => '/datatxt/cl/v1','classification_model_definition_api'=>'/datatxt/cl/models/v1','nex'=>'/datatxt/nex/v1','wikisearch'=>'/datagraph/wikisearch/v1');

  /**
   * Private constructor, we use the singleton pattern to avoid variable load
   */
  protected function __construct() {
    $this->api_endpoint = variable_get('datatxt_api_endpoint','');
    $this->app_id = variable_get('datatxt_app_id','');
    $this->app_key = variable_get('datatxt_app_key','');
  }

  /**
   * Create/reuse instance of the class
   * @return \DataTxtApiService|null
   */

  public static function getInstance() {
    if (empty(self::$instance)) {
      self::$instance = new DataTxtApiService();
    }
    return self::$instance;
  }


  /**
   * Private method that effectively calls the api.
   * Returns the response to be parsed OR FALSE otherwise
   * @param $api_endpoint , the endpoint to be called
   * @param $api_method , the method (POST/GET/PUT/DELETE)
   * @param $params , the params to be sent with the request
   * @throws \Exception, if the api endpoint is not defined
   */
  protected function _call_api($api_endpoint, $api_method, $params) {
    $params['$app_id'] = $this->app_id;
    $params['$app_key'] = $this->app_key;
    $params = drupal_http_build_query($params);
    $q = array();
    if ($api_method == 'GET') {
      $q['query'] = $params;
    }
    else {
      $q['data'] = $params;
    }
    $q['method'] = $api_method;

    $api_final_url = $this->_constructUrl($api_endpoint);
    $headers = array('Content-Type' => 'application/x-www-form-urlencoded');
    $response = drupal_http_request($api_final_url,array('headers'=>$headers, 'method'=>$api_method,'data'=> $params));
    $is_response_invalid = $this->_validateDataTXTResponse($response);
    if (!empty($is_response_invalid)) {
      watchdog('ERROR', t($is_response_invalid), array(), WATCHDOG_ERROR);
      drupal_set_message(t($is_response_invalid), 'error');
      return FALSE;
    }
    return $response->data;
  }


  /**
   * Constructs the final url for the API call
   * @param $api_endpoint , the endpoint
   * @return string, the url of the api to be called
   * @throws \Exception, if the api endpoint isn't found
   */
  private function _constructUrl($api_endpoint) {
    if (!empty(self::$api_endpoints[$api_endpoint])) {
      return $this->api_endpoint . self::$api_endpoints[$api_endpoint];
    }
    else {
      watchdog('ERROR', t('Datatxt api not found'), array(), WATCHDOG_ERROR);
      throw new Exception("The datatxt api endpoint is undefined.");
    }
  }


  /**
   * Helper that validates the response from DataTXT
   * @param $response , the response object created by the api call
   * @return bool|string, FALSE if no error occurred, the error message otherwise
   */
  private function _validateDataTXTResponse($response) {
    // If response code is different from 200, quit.
    if (isset($response->code) && $response->code > 300) {
      return "Invalid response code ({$response->code}).Error: {$response->data}";
    }
    if (!empty($response->error)) {
      return "Invalid response code ({$response->code}).Error: {$response->error}";
    }
    if (!isset($response->data)) {
      return 'Missing response data';
    }
    // No error obtained.
    return FALSE;
  }


}