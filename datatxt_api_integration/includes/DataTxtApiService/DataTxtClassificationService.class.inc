<?php

/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 23/03/15
 * Time: 13:07
 */
class DataTxtClassificationService extends DataTxtApiService {

  /**
   * Create/reuse instance of the class
   * @return \DataTxtApiService|null
   */
  public static function getIPTCClassificatorModelId() {
    return '86fab7d9-e775-4daf-838f-fcb343cc561d';
  }

  public static function getInstance() {
    if (empty(self::$instance) || get_class(self::$instance) != 'DataTxtClassificationService') {
      self::$instance = new DataTxtClassificationService();
    }
    return self::$instance;
  }

  /**
   * Classifies text content
   * @param $text , the text to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyTextContent($text, $model_id, $min_score = 0.0) {
    $params['text'] = $text;
    $params['model'] = $model_id;
    $params['min_score'] = $min_score;
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies url content
   * @param $url , the url that needs to be scraped and analyzed
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyUrlContent($url, $model_id, $min_score = 0.0) {
    $params['url'] = $url;
    $params['model'] = $model_id;
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies html content
   * @param $hmtl , the html to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyHtmlContent($html, $model_id, $min_score = 0.0) {
    $params['html'] = $html;
    $params['model'] = $model_id;
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies html fragment content
   * @param $html_fragment , the html fragment to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyHtmlFragmentContent($html_fragment, $model_id, $min_score = 0.0) {
    $params['html_fragment'] = $html_fragment;
    $params['model'] = $model_id;
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  //IPTC CALLS
  /**
   * Classifies text content
   * @param $text , the text to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyIPTCTextContent($text, $min_score = 0.0) {
    $params['text'] = $text;
    $params['model'] = self::getIPTCClassificatorModelId();
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';

    $response = $this->_call_api('iptc', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies url content
   * @param $url , the url that needs to be scraped and analyzed
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyIPTCUrlContent($url, $min_score = 0.0) {
    $params['url'] = $url;
    $params['model'] = self::getIPTCClassificatorModelId();
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('iptc', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies html content
   * @param $hmtl , the html to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyIPTCHtmlContent($html, $min_score = 0.0) {
    $params['html'] = $html;
    $params['model'] = self::getIPTCClassificatorModelId();
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Classifies html fragment content
   * @param $html_fragment , the html fragment to be classified
   * @param $model_id , the model to use to classify
   * @param float $min_score , the min score, default to zero
   * @return mixed, an array of entities found for this document
   */
  public function classifyIPTCHtmlFragmentContent($html_fragment, $min_score = 0.0) {
    $params['html_fragment'] = $html_fragment;
    $params['model'] = self::getIPTCClassificatorModelId();
    $params['min_score'] = $min_score;
    $params['include'] = 'score_details';
    $response = $this->_call_api('classification_api', 'POST', $params);
    if (!empty($response)) {
      return $this->_parseClassificationResponse($response);
    }
  }

  /**
   * Sends the model to datatxt
   * @param $model , the json representation for the model
   * @param $language , the language for this model
   * @param $description , the description for this model
   * @param $method , the action to be performed:
   * @param $model_id , the model id for update/delete actions
   *  - create: creates a new model
   *  - update: updates a model
   *  - delete: deletes a model
   */
  /*
   * Example data structure
   * {
    "lang": "The language the model will work on",
    "description": "A human-readable string you can use to describe this model",
    "categories": [
    {
    "name": "The category name",
    "topics": {
    "topic1": "weight",
    "topic2": "weight",
    "...": "...",
    }
    }
    ]
    }
   */
  public function sendClassificationModelToDataTxt($model, $method, $model_id) {
    switch ($method) {
      case 'create':
        $params['data'] = json_encode($model);
        $response = $this->_call_api('classification_model_definition_api', 'POST', $params);
        if (!empty($response)) {
          $response_data = json_decode($response);
          return $response_data->id;
        }
        else {
          watchdog('ERROR', t('Error uploading response to datatxt'), WATCHDOG_ERROR);
        }
        break;
      case 'update':
        $params['data'] = json_encode($model);
        $params['id'] = $model_id;
        $response = $this->_call_api('classification_model_definition_api', 'PUT', $params);
        if (!empty($response)) {
          $response_data = json_decode($response);
          return $response_data->id;
        }
        else {
          watchdog('ERROR', t('Error uploading response to datatxt'), WATCHDOG_ERROR);
        }
        break;
      case 'delete':
        $params['id'] = $model_id;
        $response = $this->_call_api('classification_model_definition_api', 'DELETE', $params);
        if (!empty($response)) {
          $response_data = json_decode($response);
          return $response_data->id;
        }
        else {
          watchdog('ERROR', t('Error uploading response to datatxt'), WATCHDOG_ERROR);
        }
        break;
    }
  }

  public function sendClassificationModelToDataTxtSpecifyLanguage($model, $language, $description, $method = 'create', $model_id = NULL) {
    $model->lang = $language;
    $model->description = $description;
    return $this->sendClassificationModelToDataTxt($model, $method, $model_id);
  }

  /**
   * Utility method that parses the classification response
   * @param $response_data , the data response received from datatxt
   * @return array, an array of entities found
   */
  /*
   * Example response data
   * {
    "timestamp": "Date and time of the response generation process",
    "time": "Time elapsed for generating the response (milliseconds)",
    "lang": "The language used to classify the input text (defined in the model)",
    "categories": [
    {
    "name": "The name of the category",
    "score": "The score of the category",
    "scoreDetails": {
    "entity": "URI of the entity. Only if 'include' parameter contains 'score_details'",
    "weight": "Weight of the entity. Only if 'include' parameter contains 'score_details'",
    }
    }
    ]
    }
   */

  public function datatxtWikiSearch($text, $language) {
    $params['text'] = $text;
    $params['lang'] = $language;
    $params['include'] = 'types, categories, abstract, image';
    $response = $this->_call_api('wikisearch', 'POST', $params);
    if (!empty($response)) {
      return drupal_json_decode($response);
    }
  }

  private function _parseClassificationResponse($response_data) {
    $response_object = json_decode($response_data);
    $entities = array();
    foreach ($response_object->categories as $category) {
      $entity = new DataTxtEntity();
      $entity->setEntityName($category->name);
      $entity->setEntityScore($category->score);
      if (isset($category->scoreDetails)) {
        $entity->setUriRepresentation($category->scoreDetails[0]->entity);
        foreach ($category->scoreDetails as $scoreDetail) {
          $entity->setEntityScoreDetails($scoreDetail);
        }
      }
      $entities[] = $entity;
    }
    return $entities;
  }

}
