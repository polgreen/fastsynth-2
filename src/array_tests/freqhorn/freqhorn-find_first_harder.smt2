(set-logic HORN)
(declare-fun inv-fn ((Array Int Int) Int) Bool)
(declare-var x (Array Int Int))
(declare-var c Int)
(declare-var x! (Array Int Int))
(declare-var c! Int)


(define-fun init-fn ((x (Array Int Int)) (c Int)) Bool 
	(= (select x c) 10))

(define-fun trans-fn ((x (Array Int Int)) (c Int) (x! (Array Int Int)) (c! Int)) Bool
	(forall ((index Int))  
		(and (ite (= (select x index) 10)
			(= (select x! index) 10)
			(= (select x! index ) (+ (select x index) 1)))
			(= c! c))))


(define-fun post-fn ((x (Array Int Int))(c Int)) Bool 
	(exists ((index Int)) (=> (>= index 0) (and (= (select x index) 10)
	(forall ((index2 Int)) (=> (and (>= index2 0)(< index2 index)) (not (= (select x index2) 10))))))))

(assert (forall ((x (Array Int Int))(c Int))
(=> (init-fn x c) (inv-fn x c))))
(assert (forall ((x (Array Int Int))(c Int))
(=> (and (inv-fn x c) (trans-fn x c x! c!)) (inv-fn x! c!) )))
(assert (forall ((x (Array Int Int))(c Int))
(=> (inv-fn x c) (post-fn x c))))
(check-sat)





