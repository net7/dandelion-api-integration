<?php

/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 26/03/15
 * Time: 09:56
 */
class DataTxtEntityExtractionService extends DataTxtApiService {

  /**
   * Create/reuse instance of the class
   * @return \DataTxtApiService|null
   */

  public static function getInstance() {
    if (empty(self::$instance) || get_class(self::$instance) != 'DataTxtEntityExtractionService') {
      self::$instance = new DataTxtEntityExtractionService();
    }
    return self::$instance;
  }

  public function classifyTextContent($text, $lang, $min_confidence = 0.0) {
    $params['html_fragment'] = $text;
    $params['min_confidence'] = $min_confidence;
    $params['lang'] = $lang;
    $response = $this->_call_api('nex', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseNexResponse($response, $min_confidence);
    }
  }

  public function classifyTextContentAdvanced($text, $lang, $hashtags, $min_length, $min_confidence, $social_mention, $include) {
    $params['text'] = $text;
    $params['min_confidence'] = $min_confidence;
    $params['lang'] = $lang;
    $params['social.hashtag'] = $hashtags;
    $params['min_length'] = $min_length;
    $params['social.mention'] = $social_mention;
    $params['lang'] = $lang;
    $params['include'] = $include;
    $response = $this->_call_api('nex', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseNexResponse($response, $min_confidence);
    }
  }

  private function _parseNexResponse($response_data, $min_confidence = 0) {
    $response_object = json_decode($response_data);
    $entities = array();
    foreach ($response_object->annotations as $annotation) {
      if ($annotation->confidence >= $min_confidence) {
        $entity = new DataTxtEntity();
        $entity->setEntityName($annotation->label);
        $entity->setUriRepresentation($annotation->uri);
        $entity->setEntityScore($annotation->confidence);
        $entities[] = $entity;
      }
    }
    return $entities;
  }

}