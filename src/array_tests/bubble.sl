(set-logic ALL)
;FIX ME

(declare-var a (Array Int Int))
(declare-var i Int)
(declare-var a! (Array Int Int))
(declare-var i! Int)
(declare-var swapped Int)
(declare-var swapped! Int)

(define-fun init-fn ((a (Array Int Int))(swapped Int)(i Int)) Bool 
  (and (= i 1) (= swapped 1)))


(define-fun trans-fn ((a (Array Int Int))(swapped Int)(i Int) (a! (Array Int Int))(swapped! Int)(i! Int)) Bool 
	(ite (> (select a i) (select a (bvsub i 1))) (and (= (select a! i)(select a (bvsub i 1)))
	(= (select a! (bvsub i 1))(select a i))(= i! (+ i 1)) (= swapped! 1))
	(and (= swapped! 0)(= a! a) (= i! i))))


(define-fun post-fn ((a (Array Int Int))(swapped Int)(i Int) (a! (Array Int Int))(swapped! Int)(i! Int)) Bool 
	(ite (> (select a i) (select a (bvsub i 1))) (and (= (select a! i)(select a (bvsub i 1)))
	(= (select a! (bvsub i 1))(select a i))(= i! (+ i 1)) (= swapped! 1))
	(and (= swapped! 0)(= a! a) (= i! i))))


(constraint (=> (init-fn a swapped i) (inv-fn a swapped i)))
(constraint (=> (and (inv-fn a swapped i) (trans-fn a  swapped i a! swapped! i!)) (inv-fn a! swapped! i!)))
(constraint (=> (inv-fn a swapped i) (post-fn a swapped i)))
(check-synth)