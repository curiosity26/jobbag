<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/9/14
 * Time: 12:10 PM
 */

function _jobbag_default_reminder_subject() {
  return t("[job:job_number] is approaching its due date");
}

function _jobbag_default_reminder_message() {
  return t("Job number [job:job_number] is due to be complete as of [job:job_due_date].
  Details about this job can be found at [job:url].

  Thank You");
}

function _jobbag_default_date_due_subject() {
  return t("[job:job_number] is now due date");
}

function _jobbag_default_date_due_message() {
  return t("Job number [job:job_number] is now due.
  Details about this job can be found at [job:url].

  Thank You");
}

function _jobbag_default_past_due_subject() {
  return t("[job:job_number] is past its due date!");
}

function _jobbag_default_past_due_message() {
  return t("Job number [job:job_number] is past due.
  Details about this job can be found at [job:url].

  If the due date has changed, please update the job to reflect this.

  Thank You");
}

function _jobbag_default_closed_subject() {
  return t("[job:job_number] has been closed");
}

function _jobbag_default_closed_message() {
  return t("Job number [job:job_number] has been closed.

  Great Job!

  Thank You");
}

function _jobbag_default_cancelled_subject() {
  return t("[job:job_number] has been cancelled");
}

function _jobbag_default_cancelled_message() {
  return t("Job number [job:job_number] has been cancelled.

   Thank You");
}