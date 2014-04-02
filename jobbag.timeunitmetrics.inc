<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 3/23/14
 * Time: 1:14 PM
 */

interface TimeUnitMetricInterface {
  public function toTime();
  public function fromTime($time);
}

interface TimeUnitInterface {
  public function unit($value = NULL);
  public function label($value = NULL);
  public function value($value = NULL);
}

class TimeUnitMetric implements TimeUnitMetricInterface {
  protected $unit = 'j';
  protected $metric = 0;

  public function __construct($metric = 0, $unit = 'j') {
    $this->metric($metric);
    $this->unit($unit);
  }

  static public function unitValues() {
    // Unit value keys are based off the php date() : http://php.net/manual/en/function.date.php
    return array(
      //'Y' => new TimeUnit('Y', 31536000, 'Year(s)'), // I don't think we need to go as far as years here.
      'F' => new TimeUnit('M', 1814400, 'Months(s)'),
      'W' => new TimeUnit('W', 604800, 'Week(s)'),
      'j' => new TimeUnit('j', 86400, 'Day(s)'),
      'g' => new TimeUnit('g', 3600, 'Hour(s)'),
      'i' => new TimeUnit('i', 60, 'Minutes(s)')
    );
  }

  public function toTime() {
    return $this->metric * $this->unit->value();
  }

  public function fromTime($time) {
    $units = self::unitValues();
    $timestamp = is_object($time) && $time instanceof DateTime ? $time->getTimestamp() : $time;
    foreach ($units as $unit) {
      if ($timestamp >= $unit->value()) {
        $this->unit = $unit;
        $this->metric = $timestamp / $unit->value();
        return $this;
      }
    }
    return $this;
  }

  public function metric($value = NULL) {
    if (!!$value) {
      $this->metric = $value;
    }
    return $this->metric;
  }

  public function unit($value = NULL) {
    if (!!$value) {
      $units = self::unitValues();
      $this->unit = is_object($value) && $value instanceof TimeUnit ? $value : $units[$value];
    }
    return $this->unit;
  }

  public function __get($name) {
    if (method_exists($this, $name)) {
      return $this->$name();
    }
    return NULL;
  }

  public function __set($name, $value) {
    if (method_exists($this, $name)) {
      $this->$name($value);
    }
    else {
      throw new ErrorException('Attempt to set a value of a non-existent or unavailable property on class TimeUnitMetric');
    }
  }
}

class TimeUnit implements TimeUnitInterface {
  protected $unit;
  protected $label;
  protected $value;

  public function __construct() {
    $args = func_get_args();
    if (count($args) > 0) {
      foreach ($args as $name => $value) {
        $this->$name = $value;
      }
    }
  }

  public function unit($value = NULL) {
    if (!!$value) {
      $this->unit = $value;
    }
    return $this->unit;
  }

  public function label($value = NULL) {
    if (!!$value) {
      $this->label = $value;
    }
    return $this->label;
  }

  public function value($value = NULL) {
    if (!!$value) {
      $this->value = $value;
    }
    return $this->value;
  }

  public function __set($name, $value) {
    if (method_exists($this, $name)) {
      $this->$name($value);
    }
  }

  public function __get($name) {
    if (method_exists($this, $name)) {
      return $this->$name();
    }
    return NULL;
  }

  public function __isset($name) {
    return isset($this->$name);
  }

  public function __unset($name) {
    unset($this->$name);
  }
}