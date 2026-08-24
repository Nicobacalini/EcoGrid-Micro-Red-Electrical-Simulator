
-- CREACION DE TABLAS


CREATE TABLE public.nodos (
  id_nodo integer NOT NULL,
  ubicacion character varying NOT NULL,
  tipo character varying CHECK (tipo::text = ANY (ARRAY['Consumidor'::character varying, 'Prosumidor'::character varying, 'Bateria'::character varying]::text[])),
  saldo_cuenta numeric DEFAULT 0 CHECK (saldo_cuenta >= 0::numeric),
  perfil_consumo character varying,
  CONSTRAINT nodos_pkey PRIMARY KEY (id_nodo)
);

CREATE TABLE public.lecturas_historicas (
  id_lectura integer GENERATED ALWAYS AS IDENTITY NOT NULL,
  id_nodo integer NOT NULL,
  tick_hora timestamp without time zone NOT NULL,
  produccion_kwh numeric DEFAULT 0,
  consumo_kwh numeric DEFAULT 0,
  excedente_neto numeric,
  CONSTRAINT lecturas_historicas_pkey PRIMARY KEY (id_lectura),
  CONSTRAINT lecturas_historicas_id_nodo_fkey FOREIGN KEY (id_nodo) REFERENCES public.nodos(id_nodo)
);

CREATE TABLE public.transacciones (
  id_transaccion integer GENERATED ALWAYS AS IDENTITY NOT NULL,
  id_vendedor integer NOT NULL,
  id_comprador integer NOT NULL,
  kwh numeric NOT NULL CHECK (kwh > 0::numeric),
  precio_unitario numeric NOT NULL CHECK (precio_unitario > 0::numeric),
  fecha_transaccion timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
  CONSTRAINT transacciones_pkey PRIMARY KEY (id_transaccion),
  CONSTRAINT transacciones_id_vendedor_fkey FOREIGN KEY (id_vendedor) REFERENCES public.nodos(id_nodo),
  CONSTRAINT transacciones_id_comprador_fkey FOREIGN KEY (id_comprador) REFERENCES public.nodos(id_nodo)
);

CREATE TABLE public.config_tarifas (
  hora integer NOT NULL CHECK (hora >= 0 AND hora <= 23),
  precio_base_kwh numeric NOT NULL,
  CONSTRAINT config_tarifas_pkey PRIMARY KEY (hora)
);


-- FUNCIONES Y TRIGGERS (Logica de Negocio)


CREATE OR REPLACE FUNCTION public.fn_validar_saldo()
RETURNS TRIGGER AS $$
DECLARE
    v_saldo_comprador NUMERIC;
BEGIN
    SELECT saldo_cuenta INTO v_saldo_comprador
    FROM public.nodos
    WHERE id_nodo = NEW.id_comprador;

    IF v_saldo_comprador < (NEW.kwh * NEW.precio_unitario) THEN
        RAISE EXCEPTION 'Saldo insuficiente para realizar la compra';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_validar_saldo
BEFORE INSERT ON public.transacciones
FOR EACH ROW
EXECUTE FUNCTION public.fn_validar_saldo();


-- PROCEDIMIENTOS ALMACENADOS


CREATE OR REPLACE PROCEDURE public.actualizar_saldo_y_lecturas(
    p_id_nodo INT,
    p_kwh NUMERIC,
    p_precio NUMERIC,
    p_tipo_operacion VARCHAR,
    p_tick_hora TIMESTAMP
)
LANGUAGE plpgsql
AS $$
DECLARE
    v_monto_total NUMERIC;
    v_produccion NUMERIC := 0;
    v_consumo NUMERIC := 0;
    v_excedente NUMERIC := 0;
BEGIN
    v_monto_total := p_kwh * p_precio;

    IF p_tipo_operacion = 'compra' THEN
        UPDATE public.nodos SET saldo_cuenta = saldo_cuenta - v_monto_total WHERE id_nodo = p_id_nodo;
        v_consumo := p_kwh;
        v_excedente := -p_kwh;
    ELSIF p_tipo_operacion = 'venta' THEN
        UPDATE public.nodos SET saldo_cuenta = saldo_cuenta + v_monto_total WHERE id_nodo = p_id_nodo;
        v_produccion := p_kwh;
        v_excedente := p_kwh;
    END IF;

    INSERT INTO public.lecturas_historicas (id_nodo, tick_hora, produccion_kwh, consumo_kwh, excedente_neto)
    VALUES (p_id_nodo, p_tick_hora, v_produccion, v_consumo, v_excedente);
END;
$$;


-- DATOS SEMILLA (Casos de prueba)


INSERT INTO public.config_tarifas (hora, precio_base_kwh) VALUES
(10, 1.00),
(12, 1.50),
(14, 1.00),
(15, 1.20),
(18, 2.00);

INSERT INTO public.nodos (id_nodo, ubicacion, tipo, perfil_consumo, saldo_cuenta) VALUES
(1, 'Centro', 'Consumidor', 'Residencial', 100.00),
(2, 'Sur', 'Prosumidor', NULL, 50.00),
(10, 'Parque Ind.', 'Consumidor', 'Industrial', 500.00),
(11, 'Centro', 'Consumidor', 'Comercial', 200.00),
(20, 'Norte', 'Prosumidor', NULL, 100.00),
(21, 'Este', 'Prosumidor', NULL, 80.00),
(22, 'Oeste', 'Prosumidor', NULL, 50.00),
(30, 'Sur', 'Consumidor', 'Residencial', 15.00),
(40, 'Norte', 'Prosumidor', NULL, 60.00),
(50, 'Este', 'Prosumidor', NULL, 100.00),
(51, 'Oeste', 'Consumidor', 'Residencial', 200.00),
(99, 'Plaza Central', 'Bateria', NULL, 999999.00);
