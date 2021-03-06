(set-logic HORN)
(declare-fun inv-fn ((Array Int Int)) Bool)
(declare-var x (Array Int Int))
(declare-var x! (Array Int Int))

(define-fun init-fn ((x (Array Int Int))) Bool 
	(and (exists ((index Int)) (= (select x index) 1))
	(exists ((index Int)) (= (select x index) 0))))


(define-fun trans-fn ((x (Array Int Int)) 
	(x! (Array Int Int))) Bool 
	(forall ((index Int))  
		(and (ite (= (select x index) 0)
			(= (select x! index) 1)
			(= (select x! index ) (select x index)))
			(ite (= (select x index) 1)
			(= (select x! index) 0)
			(= (select x! index ) (select x index))))))


(define-fun post-fn ((x (Array Int Int))) Bool 
	(exists ((index Int)) (= (select x index) 0)))

(assert (forall ((x (Array Int Int))
(x! (Array Int Int)))
(=> (init-fn x) (inv-fn x))))
(assert (forall ((x (Array Int Int))
(x! (Array Int Int)))
(=> (and (inv-fn x) (trans-fn x x! )) (inv-fn x! ))))
(assert (forall ((x (Array Int Int))
(x! (Array Int Int)))
(=> (inv-fn x ) (post-fn x ))))

(check-sat)

