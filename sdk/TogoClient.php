<?php
/**
 * Togo Client
 * @author zhuli
 */
class TogoClient {

	private $socket;

	public function __construct($ip, $port) {
		$this->socket =  socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ($this->socket === false) {
			throw new Exception("socket_create error!", 100);
		}
		$result = socket_connect($this->socket, $ip, $port);
		if($result === false) {
			throw new Exception("socket_connect error!", 200);
		}
	}

	public function queue_rpush($qname, $value, $priority = 0) {
		$in = "QUEUE RPUSH " . $qname . " " . $value . " " . $priority . "\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function queue_lpush($qname, $value, $priority = 0) {
		$in = "QUEUE LPUSH " . $qname . " " . $value . " " . $priority . "\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function queue_rpop($qname) {
		$in = "QUEUE RPOP " . $qname ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function queue_lpop($qname) {
		$in = "QUEUE LPOP " . $qname ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function queue_count($qname) {
		$in = "QUEUE COUNT " . $qname ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function queue_status($qname) {
		$in = "QUEUE STATUS " . $qname ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function lock_lock($name) {
		$in = "LOCK LOCK " . $name ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function lock_unlock($name) {
		$in = "LOCK UNLOCK " . $name ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function lock_status($name) {
		$in = "LOCK STATUS " . $name ."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function count_plus($name, $num = 1) {
		$in = "COUNTER PLUS " . $name ." ".$num."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function count_minus($name, $num = 1) {
		$in = "COUNTER MINUS " . $name ." ".$num."\r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}

	public function count_get($name) {
		$in = "COUNTER GET " . $name ." \r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}
	
	public function count_reset($name) {
		$in = "COUNTER RESET " . $name ." \r\n";
		socket_write($this->socket, $in, strlen($in));
		$out = socket_read($this->socket, 8192);
		$out = str_replace("TOGO_S", "", $out);
		$out = str_replace("TOGO_E", "", $out);
		return $out;
	}


	public function __destruct() {
		socket_close($this->socket);
	}
}