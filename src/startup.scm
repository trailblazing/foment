
(define-library (foment base)
    (import (foment bedrock))
    (export compile-eval compile syntax-pass middle-pass generate-pass keyword syntax
            unsyntax get-parameter set-parameter! procedure->parameter eq-hash eqv-hash
            equal-hash full-error loaded-libraries library-path full-command-line
            open-output-string get-output-string write-pretty display-pretty string-hash
            current-continuation-marks with-continuation-mark)
    (begin
        (define-syntax with-continuation-mark
            (syntax-rules ()
                ((_ key val expr) (mark-continuation key val (lambda () expr)))))))

(define-library (scheme base)
    (import (foment bedrock))
    (import (only (foment base) with-continuation-mark))
    (export quote lambda if set! let letrec letrec* let* let-values let*-values
        let-syntax letrec-syntax case or begin do syntax-rules syntax-error
        include include-ci cond-expand case-lambda quasiquote define define-values define-syntax
        import define-library else => unquote unquote-splicing
        when unless and cond call-with-values make-parameter parameterize map for-each eval
        interaction-environment boolean? not error eq? eqv? equal? command-line
        write display write-shared display-shared write-simple display-simple
        + * - / = < > <= >= zero? positive? negative? odd? even? exact-integer? expt abs sqrt
        number->string pair? cons car cdr
        set-car! set-cdr! list null? append reverse list-ref map-car map-cdr string=?
        vector? make-vector vector-ref vector-set! list->vector values apply
        call/cc (rename call/cc call-with-current-continuation) procedure? string->symbol
        caar cadr cdar cddr newline)
    (begin
        (define (caar pair) (car (car pair)))
        (define (cadr pair) (car (cdr pair)))
        (define (cdar pair) (cdr (car pair)))
        (define (cddr pair) (cdr (cdr pair)))
        (define-syntax when
            (syntax-rules ()
                ((when test result1 result2 ...)
                    (if test (begin result1 result2 ...)))))

        (define-syntax unless
            (syntax-rules ()
                ((unless test result1 result2 ...)
                    (if (not test) (begin result1 result2 ...)))))

        (define-syntax cond
            (syntax-rules (else =>)
                ((cond (else result1 result2 ...)) (begin 'ignore result1 result2 ...))
                ((cond (test => result))
                    (let ((temp test)) (if temp (result temp))))
                ((cond (test => result) clause1 clause2 ...)
                    (let ((temp test))
                        (if temp
                            (result temp)
                            (cond clause1 clause2 ...))))
                ((cond (test)) test)
                ((cond (test) clause1 clause2 ...)
                    (let ((temp test))
                        (if temp temp (cond clause1 clause2 ...))))
                ((cond (test result1 result2 ...)) (if test (begin result1 result2 ...)))
                ((cond (test result1 result2 ...) clause1 clause2 ...)
                    (if test
                        (begin result1 result2 ...)
                        (cond clause1 clause2 ...)))))

        (define (call-with-values producer consumer)
                (let-values ((args (producer))) (apply consumer args)))

        (define (make-parameter init . converter)
            (let* ((converter
                    (if (null? converter)
                        (lambda (val) val)
                        (if (null? (cdr converter))
                            (car converter)
                            (full-error 'assertion-violation 'make-parameter
                                    "make-parameter: expected one or two arguments"))))
                (init (converter init)))
            (letrec ((parameter
                        (case-lambda
                            (() (get-parameter parameter init))
                            ((val) (set-parameter! parameter (converter val)))
                            ((val ignore) (converter val)) ;; used by parameterize
                            (val (full-error 'assertion-violation '<parameter>
                                    "<parameter>: expected zero or one arguments")))))
                (procedure->parameter parameter)
                parameter)))

        (define-syntax parameterize
            (syntax-rules ()
                ((parameterize () body1 body2 ...)
                        (begin body1 body2 ...))
                ((parameterize ((param1 value1) (param2 value2) ...) body1 body2 ...)
                        (let ((p param1))
                            (if (not (parameter? p))
                                (full-error 'assertion-violation 'parameterize
                                        "parameterize: expected a parameter" p))
                            (with-continuation-mark p (p value1 'convert)
                                (parameterize ((param2 value2) ...) body1 body2 ...))))))

        (define (map proc . lists)
            (define (map proc lists)
                (let ((cars (map-car lists))
                        (cdrs (map-cdr lists)))
                    (if (null? cars)
                        '()
                        (cons (apply proc cars) (map proc cdrs)))))
            (if (null? lists)
                (full-error 'assertion-violation 'map "map: expected at least one argument")
                (map proc lists)))

        (define (for-each proc . lists)
            (define (for-each proc lists)
                (let ((cars (map-car lists))
                        (cdrs (map-cdr lists)))
                    (if (null? cars)
                        #f
                        (begin (apply proc cars) (for-each proc cdrs)))))
            (if (null? lists)
                (full-error 'assertion-violation 'for-each
                        "for-each: expected at least one argument")
                (for-each proc lists)))

        (define (eval expr env)
            ((compile-eval expr env)))))
