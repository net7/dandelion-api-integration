<?php
/**
 * Created by PhpStorm.
 * User: max2thousand
 * Date: 23/03/15
 * Time: 12:35
 */

class DataTxtEntity {

  public $entity_name;
  public $entity_score_details;
  public $entity_score;
  public $uri_representation;

  /**
   * @return mixed
   */
  public function getEntityName() {
    return $this->entity_name;
  }

  /**
   * @param mixed $entity_name
   */
  public function setEntityName($entity_name) {
    $this->entity_name = $entity_name;
  }

  /**
   * @return mixed
   */
  public function getEntityScoreDetails() {
    return $this->entity_score_details;
  }

  /**
   * @param mixed $entity_score_details
   */
  public function setEntityScoreDetails($entity_score_details) {
    $this->entity_score_details = $entity_score_details;
  }


  /**
   * @return mixed
   */
  public function getEntityScore() {
    return $this->entity_score;
  }

  /**
   * @param mixed $entity_score
   */
  public function setEntityScore($entity_score) {
    $this->entity_score = $entity_score;
  }

  /**
   * @return mixed
   */
  public function getUriRepresentation() {
    return $this->uri_representation;
  }

  /**
   * @param mixed $uri_representation
   */
  public function setUriRepresentation($uri_representation) {
    $this->uri_representation = $uri_representation;
  }



}